![Ratatoskr](https://raw.githubusercontent.com/GobanTKG/Ratatoskr-cpp/medias/medias/Ratatoskr_logo.png)
A compact Functional/Reactive/Concurrent utility library on C++17.

## inline namespace `rat::functional`

### class template `thunk<F>`

A class template that provides function composition such as map and filter. Template parameter `F` is used for inner structure, and normally you don't have to care about it. (Thanks to type or template parameter deducition.)

<table>
  <tr>
    <th>method</th>
    <th>signature</th>
    <th>description</th>
  </tr>
  <tr>
    <td rowspan="3">&lt;constructor&gt;</td>
    <td><code>()</code> -&gt; <code>thunk&lt;void&gt;</code></td>
    <td><code>thunk{}</code> constructs an empty thunk: <code>thunk&lt;void&gt;</code>.</td>
  </tr>
  <tr>
    <td><code>(const thunk&lt;F&gt; &)</code>
    <td><code>default;</code>
  </tr>
  <tr>
    <td><code>(thunk&lt;F&gt; &&)</code>
    <td><code>default;</code>
  </tr>
  <tr>
    <td>map</td>
    <td><code>(G &/&&g)</code> -&gt; <code>thunk&lt;H&gt;</code></td>
    <td>Return a new thunk that forward the invoke result of this thunk to a functor <code>g</code>. <code>g</code> must return something. (See <code>then</code>.)</td>
  </tr>
  <tr>
    <td>filter</td>
    <td><code>(G &/&&g)</code> -&gt; <code>thunk&lt;H&gt;</code></td>
    <td>Return a new thunk that immediately return <code>std::nullopt</code> if <code>g(&lt;invoke result of this thunk&gt;) == false</code>. Otherwise, the thunk return the result of this thunk.</td>
  </tr>
  <tr>
    <td>then</td>
    <td><code>(G &/&&g)</code> -&gt; <code>thunk&lt;H&gt;</code></td>
    <td>Return a new thunk, that call functor <code>g</code> by the invoke result of this thunk and then return the invoke result of this thunk. It's similar to <code>map</code>, but used to map side effect that returns <code>void</code>.</td>
  </tr>
  <tr>
    <td>try_map</td>
    <td><code>(G &/&&g)</code> -&gt; <code>thunk&lt;H&gt;<code></td>
    <td><code>g</code> must return <code>std::optional</code>. Return a new thunk that immediately return <code>std::nullopt</code> if <code>g(&lt;invoke result of this thunk&gt;) == std::nullopt</code>. Otherwise, the thunk return the <code>*g(&lt;invoke result of this thunk&gt;)</code>.</td>
  </tr>
  <tr>
    <td>operator()</td>
    <td><code>(T &/&&x)</code> -&gt; <code>std::optional&lt;R&gt;</code></td>
    <td>Invoke the composed function by an argument <code>x</code>, then return the result wrapped in <code>std::optional</code>. (Because the result can be filtered and then be none.)</td>
  </tr>
</table>

Note: In this page, universal references are referred like as `T &/&&`. 

example: 

```C++
auto is_even = [](auto n) { return n % 2 == 0; };


auto f = rat::thunk{}
           .filter(is_even)
           .then([](auto n) {
              cout << "even: " << n << endl;
           })
           .map([](auto n) { return n / 2; })
           .filter([](auto n) { return n > 5; });

f(10); // Print "even: 10", then return std::nullopt.
f(11); // Return std::nullopt.
f(12); // Print "even: 12", then return std::optional{6}.
```
### class template `bundle<Fs...>`

A class template that bundle multiple functions. (`thunk` combines functions in series, on the other hand, `bundle` combines functions in parallel.)

<table>
  <tr>
    <th>method</th>
    <th>signature</th>
    <th>description</th>
  </tr>
  <tr>
    <td rowspan="3">&lt;constructor&gt;</td>
    <td><code>(Fs... fs)</code> -&gt; <code>bundle&lt;Fs...&gt;</code></td>
    <td>Bundle multiple functions.</td>
  </tr>
  <tr>
    <td><code>(const bundle&lt;Fs...&gt; &)</code>
    <td><code>default;</code>
  </tr>
  <tr>
    <td><code>(bundle&lt;Fs...&gt; &&)</code>
    <td><code>default;</code>
  </tr>
  <tr>
    <td>bundle_with</td>
    <td><code>(Gs &/&&... gs)</code> -&gt; <code>bundle&lt;Hs...&gt;</code></td>
    <td>Return new bundle that is concatenated this bundle and functors <code>gs...</code>. If <code>gs...</code> contains some bundles, the bundles will be discomposed and rebundled.</td>
  </tr>
  <tr>
    <td>to_tuple</td>
    <td><code>()</code> -&gt; <code>std::tuple&lt;Fs...&gt;</code></td>
    <td>Convert this bundle to tuple.</td>
  </tr>
  <tr>
    <td rowspan="2">as_tuple</td>
    <td><code>() &</code> -&gt; <code>std::tuple&lt;Fs...&gt;&</code></td>
    <td rowspan="2">Get reference to this bundle as reference to tuple.</td>
  </tr>
  <tr>
    <td><code>() &&</code> -&gt; <code>std::tuple&lt;Fs...&gt;&&</code></td>
  </tr>
  <tr>
    <td>operator()</td>
    <td><code>(Ts &/&&... xs)</code> -&gt; <code>void</code></td>
    <td>Call each functions that this bundle contains one by one in order from top with arguments <code>xs...</code>.</td>
  </tr>
  <tr>
    <td>results_for</td>
    <td><code>(Ts &/&&... xs)</code> -&gt; <code>std::tuple&lt;Rs...&gt;</td>
    <td>Forward the result of invoking each functions by arguments <code>xs...</code> as tuple.There is no gurantee for invoke order.</td>
  </tr>
</table>


## inline namespace `rat::concurrent`

### class template `channel<T>`

A class template that provides inter-thread communication.
A channel can have multiple senders and only one receiver.

<table>
  <tr>
    <th>method</th>
    <th>signature</th>
    <th>description</th>
  </tr>
  <tr>
    <td>get_receiver</td>
    <td><code>()</code> -&gt; <code>receiver&lt;T&gt;</code></td>
    <td>Get the receiver. If the receiver is already got, throw <code>rat::concurrent::receiver_already_retrieved</code>.If the channel is closed, throw <code>rat::concurrent::channel_already_closed</code>.</td>
  </tr>
  <tr>
    <td>get_sender</td>
    <td><code>()</code> -&gt; <code>sender&lt;T&gt;</code></td>
    <td>Get the sender. If the channel is closed, throw <code>rat::concurrent::channel_already_closed</code>.</td>
  </tr>
  <tr>
    <td rowspan="2">push</td>
    <td><code>(const T &x)</code> -&gt; <code>void</code></td>
    <td rowspan="2">Send a value to the channel then notify the receiver.</td>
  </tr>
  <tr>
   <td><code>(T &&x)</code> -&gt; <code>void</code></td>
  </tr>
  <tr>
   <td>close</td>
   <td><code>()</code> -&gt; <code>void</code></td>
   <td>Close the channel then notify the receiver.</td>
  </tr>
</table>

### class template `sender<T>`

You can get it by `channel<T>::get_sender()`.

<table>
  <tr>
    <th>method</th>
    <th>signature</th>
    <th>description</th>
  </tr>
  <tr>
    <td rowspan="2">push</td>
    <td><code>(const T &x)</code> -&gt; <code>void</code></td>
    <td rowspan="2">Send a value to the channel then notify the receiver.</td>
  </tr>
  <tr>
   <td><code>(T &&x)</code> -&gt; <code>void</code></td>
  </tr>
  <tr>
   <td>close</td>
   <td><code>()</code> -&gt; <code>void</code></td>
   <td>Close the channel then notify the receiver.</td>
  </tr>
</table>

### class template `receiver<T>`

You can get it by `channel<T>::get_receiver()`.
The receiver is non-copyable but moveable.

<table>
  <tr>
    <th>method</th>
    <th>signature</th>
    <th>description</th>
  </tr>
  <tr>
    <td>next</td>
    <td><code>()</code> -&gt; <code>T</code></td>
    <td>Take the value sent to the channel one by one as same order as it was sent.If the channel is empty, block the thread until new value is sent.When the channel is closed, throw <code>rat::concurrent::close_channel</code>.</td>
  </tr>
  <tr>
   <td>share</td>
   <td><code>()</code> -&gt; <code>shared_receiver&lt;T&gt;</code></td>
   <td>Return shared_receiver and invalidate this receiver.</td>
  </tr>
</table>

### class template `shared_receiver<T>`

The shared_receiver is a copyable receiver.Even if there are multiple shared_receiver, one sent value will received only one time on only one shared_receiver.It is useful for processing sent value on multiple threads.

<table>
  <tr>
    <th>method</th>
    <th>signature</th>
    <th>description</th>
  </tr>
  <tr>
    <td>next</td>
    <td><code>()</code> -&gt; <code>T</code></td>
    <td>Take the value sent to the channel one by one as same order as it was sent.If the channel is empty, block the thread until new value is sent.When the channel is closed, throw <code>rat::concurrent::close_channel</code>.</td>
  </tr>
</table>

### helper function

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

```C++
using namespace std::chrono_literals;

auto [sn, rc] = rat::make_channel<int>();

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
