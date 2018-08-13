#include "../ratatoskr/functional.hpp"
#include <cstddef>
#include <iostream>

int main() {
  using namespace std;

  auto fizz_buzz = rat::bundle{
      rat::thunk{}.filter([](auto n) { return n % 3 == 0; }).then([](...) {
        cout << "Fizz";
      }),
      rat::thunk{}.filter([](auto n) { return n % 5 == 0; }).then([](...) {
        cout << "Buzz";
      }),
      rat::thunk{}
          .filter([](auto n) { return n % 3 != 0 && n % 5 != 0; })
          .then([](auto n) { cout << n; }),
      [](...) { cout << " "; }};

  auto fizz_buzz_ = rat::bundle{}.bundle_with(
      rat::thunk{}.filter([](auto n) { return n % 3 == 0; }).then([](...) {
        cout << "Fizz";
      }),
      rat::thunk{}.filter([](auto n) { return n % 5 == 0; }).then([](...) {
        cout << "Buzz";
      }),
      rat::thunk{}
          .filter([](auto n) { return n % 3 != 0 && n % 5 != 0; })
          .then([](auto n) { cout << n; }),
      [](...) { cout << endl; });

  for (size_t i = 1; i <= 20; ++i) {
    fizz_buzz(i);
    fizz_buzz_(i);
  }
}
