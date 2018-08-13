#include "./concurrent.hpp"
#include "./functional.hpp"
#include <atomic>
#include <forward_list>
#include <memory>
#include <stdexcept>
#include <thread>
#include <utility>

#ifndef RATATOSKR_REACTIVE_HPP
#define RATATOSKR_REACTIVE_HPP

namespace rat {
inline namespace reactive {

  template <class Source, class Thunk, class Finalizer>
  class signal;

  template <class Source, class Th, class... Fins>
  class signal<Source, rat::thunk<Th>, rat::bundle<Fins...>>
      : public rat::functional::detail::composable<
            signal<Source, rat::thunk<Th>, rat::bundle<Fins...>>> {

    using Self = signal<Source, rat::thunk<Th>, rat::bundle<Fins...>>;

    friend class rat::functional::detail::composable<Self>;
    template <class Source_, class Thunk_, class Finalizer_>
    friend class signal;

    template <class F>
    [[nodiscard]] auto compose(F &&f) {
      return rat::signal{std::move(receiver), std::move(closer),
                         thunk.compose(f), finalizer};
    }

    rat::receiver<Source> receiver;
    rat::channel_closer closer;

    rat::thunk<Th> thunk;
    rat::bundle<Fins...> finalizer;

    signal(rat::receiver<Source> &&rc, rat::channel_closer &&cl,
           const rat::thunk<Th> &th, const rat::bundle<Fins...> &fi)
        : receiver(std::move(rc)), closer(std::move(cl)), thunk(th),
          finalizer(fi) {}

  public:
    signal() = default;
    signal(const Self &) = delete;
    signal(Self &&) = default;
    Self &operator=(const Self &) = delete;
    signal(const rat::channel<Source> &ch)
        : receiver(ch.get_receiver()), closer(ch.get_closer()) {}

    template <class F>
    [[nodiscard]] auto finally(F f) {
      return rat::signal{std::move(receiver), std::move(closer), thunk,
                         finalizer.bundle_with(f)};
    }

    void run() {
      if (!receiver.valid()) {
        throw std::logic_error{"Running signal that has been invalid."};
      }
      if (receiver.is_closed()) {
        finalizer();
        return;
      }

      try {
        while (true) {
          thunk(receiver.next());
        }
      }
      catch (const rat::close_channel &) {
        finalizer();
      }
    }

    void run_on(rat::scheduler &sch) {
      sch.connect(
          std::thread([self = std::move(*this)]() mutable { self.run(); }),
          closer);
    }

    void run_on(rat::scheduler &sch, std::size_t parallelism) {
      if (!receiver.valid()) {
        throw std::logic_error{"Running signal that has been invalid."};
      }
      if (receiver.is_closed()) {
        sch.connect(std::thread(
            [finalizer = this->finalizer]() mutable { finalizer(); }));
        return;
      }

      auto shared_receiver = receiver.share();
      auto is_finalized = std::make_shared<std::atomic_bool>(false);
      std::forward_list<std::thread> ths;

      for (std::size_t i = 0; i < parallelism; ++i) {
        ths.push_front(
            std::thread([thunk = this->thunk, finalizer = this->finalizer,
                         shared_receiver, is_finalized]() mutable {
              try {
                while (true) {
                  thunk(shared_receiver->next());
                }
              }
              catch (const rat::close_channel &) {
                if (*is_finalized) {
                  return;
                }
                *is_finalized = true;
                finalizer();
              }
            }));
      }

      sch.connect(ths, closer);
    }
  };

  template <class Source>
  signal(rat::channel<Source>)->signal<Source, rat::thunk<void>, rat::bundle<>>;

  template <class Source, class Th, class... Fins>
  signal(rat::receiver<Source>, rat::channel_closer, rat::thunk<Th>,
         rat::bundle<Fins...>)
      ->signal<Source, rat::thunk<Th>, rat::bundle<Fins...>>;

} // namespace reactive
} // namespace rat
#endif
