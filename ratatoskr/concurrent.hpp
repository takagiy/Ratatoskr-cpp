#include <condition_variable>
#include <forward_list>
#include <memory>
#include <mutex>
#include <optional>
#include <thread>
#include <utility>

#ifndef RATATOSKR_CONCURRENT_HPP
#define RATATOSKR_CONCURRENT_HPP

namespace rat {
inline namespace concurrent {
  namespace detail {
    template <class T>
    struct channel_state;

    template <class T>
    struct channel_state {
      bool has_receiver_v;
      bool is_closed_v;
      std::forward_list<std::optional<T>> data;
      mutable std::mutex data_mutex;
      typename std::forward_list<std::optional<T>>::iterator last;
      std::condition_variable notifier;

      channel_state()
          : is_closed_v(false), data{std::nullopt}, last(data.begin()) {}

      channel_state(const channel_state &) = delete;
      channel_state(channel_state &&) = delete;
    };
  } // namespace detail

  template <class T>
  class channel;

  template <class T>
  class sender;

  template <class T>
  class receiver;

  class channel_closer;

  class scheduler;

  class close_channel : public std::exception {
    using std::exception::exception;
    auto what() const noexcept -> const char * { return "close_channel"; }
  };

  class channel_already_closed : public std::logic_error {
    using std::logic_error::logic_error;
  };

  class receiver_already_retrived : public std::logic_error {
    using std::logic_error::logic_error;
  };

  struct sharing_receiver_t {
    explicit sharing_receiver_t() = default;
  };

  inline constexpr sharing_receiver_t sharing_receiver = sharing_receiver_t();

  class channel_closer {
    template <class T>
    friend class channel;

    template <class T>
    channel_closer(const std::shared_ptr<detail::channel_state<T>> &state)
        : reference_counter(state), is_closed_v(state->is_closed_v),
          data_mutex(state->data_mutex), notifier(state->notifier) {}
    std::shared_ptr<void> reference_counter;
    bool &is_closed_v;
    std::mutex &data_mutex;
    std::condition_variable &notifier;

  public:
    channel_closer() = default;
    channel_closer(const channel_closer &) = default;
    channel_closer(channel_closer &&) = default;

    auto valid() const -> bool { return static_cast<bool>(reference_counter); }

    void close() const {
      {
        std::lock_guard lock{data_mutex};
        is_closed_v = true;
      }
      notifier.notify_all();
    }
  };

  template <class T>
  class channel {
    std::shared_ptr<detail::channel_state<T>> state;

  public:
    using receiver_type = receiver<T>;
    using sender_type = sender<T>;
    using value_type = T;

    channel() : state(std::make_shared<detail::channel_state<T>>()) {}
    channel(const channel<T> &) = default;
    channel(channel<T> &&) = default;

    auto get_sender() const -> sender<T> { return sender<T>{state}; }
    auto get_receiver() const -> receiver<T> { return receiver<T>{state}; }
    auto get_closer() const -> channel_closer { return channel_closer{state}; }

    auto valid() const -> bool { return static_cast<bool>(state); }

    void push(const T &x) {
      {
        std::lock_guard lock{state->data_mutex};
        if (!state->has_receiver_v) {
          return;
        }
        state->data.insert_after(state->last, x);
        ++state->last;
      }
      state->notifier.notify_one();
    }
    void push(T &&x) {
      {
        std::lock_guard lock{state->data_mutex};
        if (!state->has_receiver_v) {
          return;
        }
        state->data.insert_after(state->last, std::move(x));
        ++state->last;
      }
      state->notifier.notify_one();
    }
    void close() {
      {
        std::lock_guard lock{state->data_mutex};
        state->is_closed_v = true;
      }
      state->notifier.notify_all();
    }

    bool is_closed() const {
      std::lock_guard{state->data_mutex};
      return state->is_closed_v;
    }
  };

  template <class T>
  class sender {
    template <class T_>
    friend class channel;

    std::shared_ptr<detail::channel_state<T>> state;

    sender(const std::shared_ptr<detail::channel_state<T>> &state)
        : state(state) {
      std::lock_guard lock{state->data_mutex};
      if (state->is_closed_v) {
        throw channel_already_closed{"sender::sender"};
      }
    }

  public:
    using channel_type = channel<T>;
    using value_type = T;

    sender() = default;
    sender(const sender<T> &) = default;
    sender(sender<T> &&) = default;

    auto valid() const -> bool { return static_cast<bool>(state); }

    void push(const T &x) {
      {
        std::lock_guard lock{state->data_mutex};
        if (!state->has_receiver_v) {
          return;
        }
        state->data.insert_after(state->last, x);
        ++state->last;
      }
      state->notifier.notify_one();
    }
    void push(T &&x) {
      {
        std::lock_guard lock{state->data_mutex};
        if (!state->has_receiver_v) {
          return;
        }
        state->data.insert_after(state->last, std::move(x));
        ++state->last;
      }
      state->notifier.notify_one();
    }
    void close() {
      {
        std::lock_guard lock{state->data_mutex};
        state->is_closed_v = true;
      }
      state->notifier.notify_all();
    }
    auto is_closed() const -> bool {
      std::lock_guard{state->data_mutex};
      return state->is_closed_v;
    }
  };

  template <class T>
  class receiver {
    template <class T_>
    friend class channel;

    std::shared_ptr<detail::channel_state<T>> state;
    typename std::forward_list<std::optional<T>>::iterator iterator;

    receiver(const std::shared_ptr<detail::channel_state<T>> &state)
        : state(state),
          iterator((std::lock_guard{state->data_mutex}, state->last)) {
      std::lock_guard lock{state->data_mutex};
      if (state->has_receiver_v) {
        throw receiver_already_retrived{"receiver::receiver"};
      }
      else if (state->is_closed_v) {
        throw channel_already_closed{"receiver::receiver"};
      }
      else {
        state->has_receiver_v = true;
      }
    }

  public:
    using channel_type = channel<T>;
    using value_type = T;

    receiver() = default;
    receiver(const receiver &) = delete;
    receiver &operator=(const receiver &) = delete;
    receiver(receiver &&) = default;
    receiver &operator=(receiver &&) = default;

    auto valid() const -> bool { return static_cast<bool>(state); }

    auto next() -> T {
      std::unique_lock lock{state->data_mutex};
      state->notifier.wait(lock, [this] {
        auto next = iterator;
        return ++next != state->data.end() || state->is_closed_v;
      });

      if (state->is_closed_v) {
        throw close_channel{};
      }

      ++iterator;
      state->data.pop_front();
      return **iterator;
    }

    auto operator*() -> std::optional<T> {
      std::lock_guard lock{state->data_mutex};
      return *iterator;
    }

    auto share() -> std::shared_ptr<receiver<T>> {
      return std::make_shared<receiver<T>>(std::move(*this));
    }

    auto is_closed() const -> bool {
      std::lock_guard{state->data_mutex};
      return state->is_closed_v;
    }
  };

  class scheduler {
    std::forward_list<rat::channel_closer> closers;
    std::forward_list<std::thread> threads;
    bool is_closed_v;
    mutable std::mutex m;
    mutable std::condition_variable cv;

  public:
    scheduler() : is_closed_v(false) {}
    scheduler(const scheduler &) = delete;
    scheduler &operator=(const scheduler &) = delete;
    scheduler(scheduler &&) = delete;
    scheduler &operator=(scheduler &&) = delete;

    void halt() {
      {
        std::lock_guard lock{m};
        is_closed_v = true;
        for (auto &&c : closers) {
          c.close();
        }
      }
      cv.notify_all();
    }

    void connect(std::thread &&th) {
      std::lock_guard lock{m};
      if (is_closed_v) {
        th.join();
        return;
      }
      threads.emplace_front(std::move(th));
    }

    void connect(std::thread &&th, const rat::channel_closer &closer) {
      std::lock_guard lock{m};
      if (is_closed_v) {
        closer.close();
        th.join();
        return;
      }
      threads.emplace_front(std::move(th));
      closers.push_front(closer);
    }
    void connect(std::thread &&th, rat::channel_closer &&closer) {
      std::lock_guard lock{m};
      if (is_closed_v) {
        closer.close();
        th.join();
        return;
      }
      threads.emplace_front(std::move(th));
      closers.push_front(std::move(closer));
    }

    void connect(std::forward_list<std::thread> &ths,
                 const rat::channel_closer &closer) {
      std::lock_guard lock{m};
      if (is_closed_v) {
        closer.close();
        for (auto &&th : ths) {
          th.join();
        }
        return;
      }
      threads.splice_after(threads.before_begin(), ths);
      closers.push_front(closer);
    }
    void connect(std::forward_list<std::thread> &&ths,
                 const rat::channel_closer &closer) {
      std::lock_guard lock{m};
      if (is_closed_v) {
        closer.close();
        for (auto &&th : ths) {
          th.join();
        }
        return;
      }
      threads.splice_after(threads.before_begin(), ths);
      closers.push_front(closer);
    }

    void wait() {
      std::unique_lock lock{m};
      cv.wait(lock, [this] { return is_closed_v; });
      for (auto &&th : threads) {
        th.join();
      }
    }
  };

  template <class T>
  auto make_channel() {
    channel<T> ch;
    return std::pair{ch.get_sender(), ch.get_receiver()};
  }

  template <class T>
  auto make_channel(sharing_receiver_t) {
    channel<T> ch;
    return std::pair{ch.get_sender(), ch.get_receiver().share()};
  }

} // namespace concurrent
} // namespace rat
#endif
