#include "./concurrent.hpp"
#include "./functional.hpp"
#include "./trvial.hpp"
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
                         thunk.compose(std::forward<F>(f)), finalizer};
    }

    typename Source::receiver_type receiver;
    rat::channel_closer closer;

    rat::thunk<Th> thunk;
    rat::bundle<Fins...> finalizer;

    signal(typename Source::receiver_type &&rc, rat::channel_closer &&cl,
           const rat::thunk<Th> &th, const rat::bundle<Fins...> &fi)
        : receiver(std::move(rc)), closer(std::move(cl)), thunk(th),
          finalizer(fi) {}

  public:
    using value_type =
        decltype(thunk(std::declval<typename Source::value_type>()));

    signal() = default;
    signal(const Self &) = delete;
    signal(Self &&) = default;
    Self &operator=(const Self &) = delete;
    signal(const Source &ch)
        : receiver(ch.get_receiver()), closer(ch.get_closer()) {}

    template <class F>
    [[nodiscard]] auto finally(F &&f) {
      return rat::signal{std::move(receiver), std::move(closer), thunk,
                         finalizer.bundle_with(std::forward<F>(f))};
    }

    void run() {
      if (!receiver.valid()) {
        throw std::logic_error{"Running signal that has been invalid."};
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

    auto next() -> std::optional<value_type> {
      try {
        return std::optional{receiver.next()};
      }
      catch (const rat::close_channel &) {
        return std::nullopt;
      }
    }
  };

  template <class Source>
  signal(Source)->signal<Source, rat::thunk<void>, rat::bundle<>>;

  template <class Receiver, class Th, class... Fins>
  signal(Receiver, rat::channel_closer, rat::thunk<Th>, rat::bundle<Fins...>)
      ->signal<typename Receiver::channel_type, rat::thunk<Th>,
               rat::bundle<Fins...>>;

  template <class... Sources, class Th, class... Fins>
  class signal<std::tuple<Sources...>, rat::thunk<Th>, rat::bundle<Fins...>>
      : public rat::functional::detail::composable<signal<
            std::tuple<Sources...>, rat::thunk<Th>, rat::bundle<Fins...>>> {

    using Self =
        signal<std::tuple<Sources...>, rat::thunk<Th>, rat::bundle<Fins...>>;

    friend class rat::functional::detail::composable<Self>;
    template <class Source_, class Thunk_, class Finalizer_>
    friend class signal;

    template <class F>
    [[nodiscard]] auto compose(F &&f) {
      return rat::signal{std::move(sources), thunk.compose(std::forward<F>(f)),
                         finalizer};
    }

    rat::thunk<Th> thunk;
    rat::bundle<Fins...> finalizer;

    std::tuple<Sources...> sources;
    std::tuple<typename Sources::value_type...> values;

    signal(std::tuple<Sources...> &&srcs, const rat::thunk<Th> &th,
           const rat::bundle<Fins...> &fi)
        : sources(std::move(srcs)), thunk(th), finalizer(fi) {}

  public:
    signal() = default;
    signal(const Self &) = delete;
    signal(Self &&) = default;
    Self &operator=(Self &&) = delete;

    signal(std::tuple<Sources...> &&srcs) : sources(std::move(srcs)) {}

    template <class F>
    [[nodiscard]] auto finally(F &&f) {
      return rat::signal{std::move(sources), thunk,
                         finalizer.bundle_with(std::forward<F>(f))};
    }

    void run_on(rat::scheduler &sch) {
      auto is_finalized = std::make_shared<std::atomic_bool>(false);

      rat::tuples::for_each(sources, [&sch](auto &&src) {
        sch.connect(
            std::thread([thunk = src.thunk, finalizer = src.finalizer]() {}),
            src.closer);
      });
    }
  };

  template <class... Sources, class Th, class... Fins>
  signal(std::tuple<Sources...>, rat::thunk<Th>, rat::bundle<Fins...>)
      ->signal<std::tuple<Sources...>, rat::thunk<Th>, rat::bundle<Fins...>>;

} // namespace reactive
} // namespace rat
#endif
