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


