#include <condition_variable>
#include <forward_list>
#include <memory>
#include <mutex>
#include <optional>
#include <utility>

#ifndef RATATOSKR_CONCURRENT_HPP
#define RATATOSKR_CONCURRENT_HPP

namespace ratatoskr::concurrent {
template <class T>
struct channel_state;

template <class T>
class channel;

template <class T>
class sender;

template <class T>
class receiver;

template <class T>
class shared_receiver;

class close_channel : public std::exception {
  const char *what() const noexcept { return "close_channel"; }
};

class channel_already_closed : public std::logic_error {
  using std::logic_error::logic_error;
};

class receiver_already_retrived : public std::logic_error {
  using std::logic_error::logic_error;
};

template <class T>
struct channel_state {
  bool has_receiver_v;
  bool is_closed_v;
  std::forward_list<T> data;
  mutable std::mutex data_mutex;
  typename std::forward_list<T>::iterator last;
  std::condition_variable notifier;

  channel_state() : is_closed_v(false), last(data.before_begin()) {}
};

template <class T>
class channel {
  std::shared_ptr<channel_state<T>> state;

public:
  channel() : state(std::make_shared<channel_state<T>>()) {}

  sender<T> get_sender() { return sender<T>{state}; }
  receiver<T> get_receiver() { return receiver<T>{state}; }
};

template <class T>
class sender {
  std::shared_ptr<channel_state<T>> state;

public:
  sender(const std::shared_ptr<channel_state<T>> &state) : state(state) {
    std::lock_guard lock{state->data_mutex};
    if (state->is_closed_v) {
      throw channel_already_closed{"sender::sender"};
    }
  }

  bool avail() const { return state.use_count() != 0; }

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
};

template <class T>
class receiver {
  std::shared_ptr<channel_state<T>> state;
  std::optional<typename std::forward_list<T>::iterator> iterator;

public:
  receiver(const std::shared_ptr<channel_state<T>> &state) : state(state) {
    std::lock_guard lock{state->data_mutex};
    if (state->has_receiver_v) {
      throw receiver_already_retrived{"receiver::receiver"};
    }
    else if (state->is_closed_v) {
      throw channel_already_closed{"receiver::receiver"};
    }
    else {
      iterator = state->last;
      state->has_receiver_v = true;
    }
  }

  receiver(const receiver &) = delete;
  receiver &operator=(const receiver &) = delete;
  receiver(receiver &&) = default;
  receiver &operator=(receiver &&) = default;

  bool avail() const { return state.use_count() != 0; }

  T next() {
    std::unique_lock lock{state->data_mutex};
    state->notifier.wait(lock, [this] {
      auto next = *iterator;
      return ++next != state->data.end() || state->is_closed_v;
    });

    if (state->is_closed_v) {
      throw close_channel{};
    }

    ++*iterator;
    return **iterator;
  }
};

template

    template <class T>
    auto make_channel() {
  channel<T> ch;
  return std::pair{ch.get_sender(), ch.get_receiver()};
}

} // namespace ratatoskr::concurrent

#endif
