![Ratatoskr](https://raw.githubusercontent.com/GobanTKG/Ratatoskr-cpp/medias/medias/Ratatoskr_logo.png)
## A compact Functional/Reactive/Concurrent utilitiy library on C++17

## `ratatoskr::functional::thunk<F>`
* A class template that provides map or filter function composition.
* The invoke result is wrapped in std::optional.
* Every method donesn't make any change to the original thunk but return a new thunk.

example:

```cpp
auto even = [](auto n) { retrun n % 2 == 0; };

auto f = thunk{}
           .filter(even)
           .map([](auto n) { return n / 2; })
           .map([](auto n) {
             cout << n << endl;
             return n;
           });

f(0) //=> print 0. return optional(0).
f(1) //=> return nullopt.
f(2) //=> print 1. return optional(1).
```

## `ratatoskr::concurrent::channel<T>`
* A class template that provides inter-thread communication.
* A channel is able to have some senders and only one receiver.
* `channel::get_sender()` 
    If the channel is already closed, throw `ratatoskr::concurrent::channel_already_closed`.
* `channel::get_receiver()`
    If the channel is already closed, throw `ratatoskr::concurrent::channel_already_closed`.
    If the receiver is already got, throw `ratatoskr::concurrent::receiver_already_retrieved`.

## `ratatoskr::concurrent::sender<T>`
* `sender::push(const T & / T &&)`
    Send an object to the channel.
* `sender::close()`
    Close the channel then notify the receiver.

## `ratatoskr::concurrent::receiver<T>`
* Non-copyable but movable.
* `receiver::next() -> T`
    Take the data that was sent to the channel in the same order as it was sent. If the channel is empty, block the thread until the new data is sent. When the channel is closed, throw `ratatoskr::concurrent::close_channel`.

## `ratatoskr::make_channel<T>() -> std::pair<sender<T>, receiver<T>>`
* A helper function that creates pair of sender and receiver.

example:

```cpp
auto [sn, rc] = make_channel<int>();

auto produce = [](auto sn) {
  for (int i = 0; i < 10; ++i) {
    sn.push(i);
    std::this_thread::sleep_for(1s);
  }
  sn.close();
};

auto consume = [](auto rc) {
  try {
    while (true) {
      std::cout << rc.next() << std::endl;
    }
  }
  catch (const close_channel &_) {
  }
};

std::thread producer{produce, std::move(sn)};
std::thread consumer{consume, std::move(rc)};
producer.join();
consumer.join();
```
