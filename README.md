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
    <td rowspan="4"><code>thunk&lt;F&gt;</code></td>
    <td>map</td>
    <td><code>(G g)</code> -&gt; <code>thunk&lt;H&gt;</code></td>
    <td>Return a new thunk compounded a map functor g.</td>
  </tr>
  <tr>
    <td>filter</td>
    <td><code>(G g)</code> -&gt; <code>thunk&lt;H&gt;</code></td>
    <td>Return a new thunk compounded a filter functor g.</td>
  </tr>
  <tr>
    <td rowspan="2">operator()</td>
    <td><code>(const T &x)</code> -&gt; <code>std::optional<R></code></td>
    <td rowspan="2">Invoke the composed function passing an argument x then return the result wrapped in <code>std::optional</code>.</td>
  </tr>
  <tr>
    <td><code>(T &&x)</code> -&gt; <code>std::optional<R></code></td>
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
           .filter([](auto n) { return n > 5; });

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
    <td rowspan="5"><code>channel&lt;T&gt;</code></td>
    <td>get_receiver</td>
    <td><code>()</code> -&gt; <code>receiver&lt;T&gt;</code></td>
    <td>Get the receiver. If the receiver is already got, throw <code>ratatoskr::concurrent::receiver_alredy_retrieved</code>.If the chanell is closed, throw <code>ratatoskr::concurrent::channel_already_closed</code>.</td>
  </tr>
  <tr>
    <td>get_sender</td>
    <td><code>()</code> -&gt; <code>sender&lt;T&gt;</code></td>
    <td>Get the sender. If the channel is closed, throw <code>ratatoskr::concurrent::channel_alredy_closed</code>.</td>
  </tr>
  <tr>
    <td rowspan="2">push</td>
    <td><code>(const T &x)</code> -&gt; <code>void</code></td>
    <td rowspan="2">Send a value to the channel.</td>
  </tr>
  <tr>
   <td><code>(T &&x)</code> -&gt; <code>void</code></td>
  </tr>
  <tr>
   <td>close</td>
   <td><code>()</code> -&gt; <code>void</code></td>
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
    <td rowspan="3"><code>sender&lt;T&gt;</code></td>
    <td rowspan="2">push</td>
    <td><code>(const T &x)</code> -&gt; <code>void</code></td>
    <td rowspan="2">Send a value to the channel.</td>
  </tr>
  <tr>
   <td><code>(T &&x)</code> -&gt; <code>void</code></td>
  </tr>
  <tr>
   <td>close</td>
   <td><code>()</code> -&gt; <code>void</code></td>
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
    <td rowspan="2"><code>reciever&lt;T&gt;</code></td>
    <td>next</td>
    <td><code>()</code> -&gt; <code>T</code></td>
    <td>Take the value sent to the channel one by one as same order as it was sent.If the channel is empty, block the thread until new value is sent.When the channel is closed, throw <code>ratatoskr::concurrent::close_channel</code>.</td>
  </tr>
  <tr>
   <td>share</td>
   <td><code>()</code> -&gt; <code>shared_receiver&lt;T&gt;</code></td>
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
    <td rowspan="2"><code>make_channel&lt;T&gt;</code></td>
    <td><code>()</code> -&gt; <code>std::pair&lt;sender&lt;T&gt;, receiver&lt;T&gt;&gt;</code></td>
    <td>Create a pair of <code>sender&lt;T&gt;</code> and <code>receiver&lt;T&gt;</code>.</td>
  </tr>
  <tr>
    <td><code>(with_shared_receiver_t)</code> -&gt; <code>std::pair&lt;sender&lt;T&gt;, shared_receiver&lt;T&gt;&gt;</code></td>
    <td>When passed <code>with_shared_receiver</code>, share the receiver.</td>
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
