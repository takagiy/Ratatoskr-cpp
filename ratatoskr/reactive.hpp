#include <atomic>
#include <forward_list>
#include <memory>
#include <ratatoskr/concurrent.hpp>
#include <ratatoskr/functional.hpp>
#include <thread>
#include <tuple>
#include <utility>

#ifndef RATATOSKR_REACTIVE_HPP
#define RATATOSKR_REACTIVE_HPP

namespace rat {
inline namespace reactive {
  template <class T>
  class source;

  template <class T, class Thunk, class... Finally>
  class signal;

  template <class T>
  class source {
    rat::channel<T> ch;

  public:
    source() {}

    auto signal() { return rat::signal{ch, rat::thunk{}, std::tuple<>{}}; }

    auto get_closer() { return ch.get_closer(); }

    auto get_sender() { return ch.get_sender(); }

    template <class F>
    [[nodiscard]] auto map(F &&f) {
      return rat::signal{ch, rat::thunk{}.map(std::forward<F>(f)),
                         std::tuple<>{}};
    }

    template <class F>
    [[nodiscard]] auto try_map(F &&f) {
      return rat::signal{ch, rat::thunk{}.try_map(std::forward<F>(f)),
                         std::tuple<>{}};
    }

    template <class F>
    [[nodiscard]] auto then(F &&f) {
      return rat::signal{ch, rat::thunk{}.then(std::forward<F>(f)),
                         std::tuple<>{}};
    }

    template <class F>
    [[nodiscard]] auto filter(F &&f) {
      return rat::signal{ch, rat::thunk{}.filter(std::forward<F>(f)),
                         std::tuple<>{}};
    }

    template <class F>
    [[nodiscard]] auto finally(F &&f) {
      return rat::signal{ch, rat::thunk{}, std::tuple{std::forward<F>(f)}};
    }

    void push(const T &x) { ch.push(x); }
    void push(T &&x) { ch.push(std::move(x)); }

    void close() { ch.close(); }
  };

  template <class T, class Fs, class... Finally>
  class signal {
    template <class T_>
    friend class source;

    template <class T_, class Fs_, class... Finally_>
    friend class signal;

    rat::channel<T> ch;
    rat::thunk<Fs> thunk;
    std::tuple<Finally...> finalizer;
    std::shared_ptr<std::atomic_bool> is_finalized_p;

    template <size_t... I>
    void call_finalizer(std::index_sequence<I...>) {
      (std::get<I>(finalizer)(), ...);
    }

    void finalize() {
      if (*is_finalized_p)
        return;
      *is_finalized_p = true;
      call_finalizer(std::make_index_sequence<sizeof...(Finally)>{});
    }

    template <class Channel, class Thunk, class Tuple,
              std::enable_if_t<
                  (std::is_constructible_v<rat::channel<T>, Channel &&> &&
                   std::is_constructible_v<rat::thunk<Fs>, Thunk &&> &&
                   std::is_constructible_v<std::tuple<Finally...>, Tuple &&>),
                  std::nullptr_t> = nullptr>
    signal(Channel &&ch, Thunk &&thunk, Tuple &&finalizer)
        : ch(std::forward<Channel>(ch)), thunk(std::forward<Thunk>(thunk)),
          finalizer(std::forward<Tuple>(finalizer)),
          is_finalized_p(std::make_shared<std::atomic_bool>(false)) {}

  public:
    void run() {
      if (!ch.is_closed()) {
        static auto rc = ch.get_receiver();

        try {
          while (true) {
            thunk(rc.next());
          }
        }
        catch (const rat::close_channel &) {
          finalize();
        }
      }
      else {
        finalize();
      }
    }

    auto next() -> std::optional<decltype(thunk(std::declval<T>()))> {
      if (!ch.is_closed()) {
        static auto rc = ch.get_receiver();

        try {
          return std::optional{thunk(rc.next())};
        }
        catch (const rat::close_channel &) {
          finalize();
          return std::nullopt;
        }
      }
      else {
        finalize();
        return std::nullopt;
      }
    }

    template <class F>
    [[nodiscard]] auto map(F &&f) & {
      return rat::signal{ch, thunk.map(std::forward<F>(f)), finalizer};
    }
    template <class F>
    [[nodiscard]] auto map(F &&f) && {
      return rat::signal{std::move(ch), thunk.map(std::forward<F>(f)),
                         std::move(finalizer)};
    }

    template <class F>
    [[nodiscard]] auto try_map(F &&f) & {
      return rat::signal{ch, thunk.try_map(std::forward<F>(f)), finalizer};
    }
    template <class F>
    [[nodiscard]] auto try_map(F &&f) && {
      return rat::signal{std::move(ch), thunk.try_map(std::forward<F>(f)),
                         std::move(finalizer)};
    }

    template <class F>
    [[nodiscard]] auto then(F &&f) & {
      return rat::signal{ch, thunk.then(std::forward<F>(f)), finalizer};
    }
    template <class F>
    [[nodiscard]] auto then(F &&f) && {
      return rat::signal{std::move(ch), thunk.then(std::forward<F>(f)),
                         std::move(finalizer)};
    }

    template <class F>
    [[nodiscard]] auto filter(F &&f) & {
      return rat::signal{ch, thunk.filter(std::forward<F>(f)), finalizer};
    }
    template <class F>
    [[nodiscard]] auto filter(F &&f) && {
      return rat::signal{std::move(ch), thunk.filter(std::forward<F>(f)),
                         std::move(finalizer)};
    }

    template <class F>
    [[nodiscard]] auto finally(F &&f) & {
      return rat::signal{
          ch, thunk, std::tuple_cat(finalizer, std::tuple{std::forward<F>(f)})};
    }
    template <class F>
    [[nodiscard]] auto finally(F &&f) && {
      return rat::signal{
          std::move(ch), std::move(thunk),
          std::tuple_cat(finalizer, std::tuple{std::forward<F>(f)})};
    }

    void run_on(rat::scheduler &sch) {
      sch.connect(std::thread([*this]() mutable { this->run(); }),
                  ch.get_closer());
    }

    void run_on(rat::scheduler &sch, std::size_t n) {
      std::forward_list<std::thread> ths;
      for (std::size_t i = 0; i < n; ++i) {
        ths.emplace_front([*this]() mutable { this->run(); });
      }
      sch.connect(ths, ch.get_closer());
    }
  };

  template <class T, class Fs, class... Finally>
  signal(rat::channel<T>, rat::thunk<Fs>, std::tuple<Finally...>)
      ->signal<T, Fs, Finally...>;

} // namespace reactive
} // namespace rat

#endif
