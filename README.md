![Ratatoskr](https://raw.githubusercontent.com/GobanTKG/Ratatoskr-cpp/medias/medias/Ratatoskr_logo.png)
## A compact Functional/Reactive/Concurrent utilitiy library on C++17

## namespace `ratatoskr::functional`

<table>
  <tr>
    <th>class</th>
    <th>method</th>
    <th>signature</th>
    <th>description</th>
  </tr>
  <tr>
    <td rowspan="4">`thunk<F>`</td>
    <td>map</td>
    <td>`(G g)` -&gt; `thunk<H>`</td>
    <td>Return a new thunk compounded a map functor g.</td>
  </tr>
  <tr>
    <td>filter</td>
    <td>`(G g)` -&gt; `thunk<H>`</td>
    <td>Return a new thunk compounded a filter functor g.</td>
  </tr>
  <tr>
    <td rowspan="2">operator()</td>
    <td>`(const T &x)` -&gt; `std::optional<R>`</td>
    <td rowspan="2">Invoke the composed function passing an argument x then return the result wrapped in `std::optional`.</td>
  </tr>
  <tr>
    <td>`(T &&x)` -&gt; `std::optional<R>`</td>
  </tr>
</table>

example: 

```c++
using namespace::ratatoskr::functional;

auto even = [](auto n) { return n % 2 == 0; };


auto f = thunk{}
           .filter(even)
           .map([](auto n) {
              cout << n << endl;
              return n;
           })
           .map([](auto n) { return n / 2; })
           .filter([](auto n) {
             return n > 5;
           });

f(10); // Print "10", then return std::nullopt.
f(11); // Return std::nullopt.
f(12); // Print "12", then return std::optional{6}.
```


## namespace `ratatoskr::concurrent`

<table>
  <tr>
    <th>class</th>
    <th>method</th>
    <th>signature</th>
    <th>description</th>
  </tr>
  <tr>
    <td rowspan="5">`channel<T>`</td>
    <td>get_receiver</td>
    <td>`()` -&gt; `receiver<T>`</td>
    <td>Get the receiver. If the receiver is already got, throw `ratatoskr::concurrent::receiver_alredy_retrieved`.If the chanell is closed, throw `ratatoskr::concurrent::channel_already_closed`.</td>
  </tr>
  <tr>
    <td>get_sender</td>
    <td>`()` -&gt; `sender<T>`</td>
    <td>Get the sender. If the channel is closed, throw `ratatoskr::concurrent::channel_alredy_closed`.</td>
  </tr>
  <tr>
    <td rowspan="2">push</td>
    <td>`(const T &x)` -&gt; `void`</td>
    <td rowspan="2">Send a value to the channel.</td>
  </tr>
  <tr>
   <td>`(T &&x)` -&gt; `void`</td>
  </tr>
  <tr>
   <td>close</td>
   <td>`void` -&gt; `void`</td>
   <td>Close the channel.</td>
  </tr>
</table>

<table>
  <tr>
    <th>class</th>
    <th>method</th>
    <th>signature</th>
    <th>description</th>
  </tr>
  <tr>
    <td rowspan="3">`sender<T>`</td>
    <td rowspan="2">push</td>
    <td>`(const T &x)` -&gt; `void`</td>
    <td rowspan="2">Send a value to the channel.</td>
  </tr>
  <tr>
   <td>`(T &&x)` -&gt; `void`</td>
  </tr>
  <tr>
   <td>close</td>
   <td>`()` -&gt; `void`</td>
   <td>Close the channel.</td>
  </tr>
</table>

<table>
  <tr>
    <th>class</th>
    <th>method</th>
    <th>signature</th>
    <th>description</th>
  </tr>
  <tr>
    <td rowspan="2">`reciever<T>`</td>
    <td>next</td>
    <td>`()` -&gt; `T`</td>
    <td>Take the value sent to the channel one by one as same order as it was sent.If the channel is empty, block the thread until new value is sent.When the channel is closed, throw `ratatoskr::concurrent::close_channel`.</td>
  </tr>
  <tr>
   <td>share</td>
   <td>`()` -&gt; `shared_receiver<T>`</td>
   <td>Return shared_receier and invalidate this receier.</td>
  </tr>
</table>

<table>
  <tr>
    <th>function</th>
    <th>signature</th>
    <th>description</th>
  </tr>
  <tr>
    <td rowspan="2">`make_channel<T>`</td>
    <td>`()` -&gt; `std::pair<sender<T>, receiver<T>>`</td>
    <td>Create a pair of `sender<T>` and `receiver<T>`.</td>
  </tr>
  <tr>
    <td>`(with_shared_receiver_t)` -&gt; `std::pair<sender<T>, shared_receiver<T>>`</td>
    <td>When passed `with_shared_receiver`, share the receiver.</td>
  </tr>
<table>

example:

```c++
using namespace ratatoskr::concurrent;
using namespace std::chrono_literals;

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
  catch (const close_channel &) {
  }
};

std::thread producer{produce, std::move(sn)};
std::thread consumer{consume, std::move(rc)};
producer.join();
consumer.join();
```
