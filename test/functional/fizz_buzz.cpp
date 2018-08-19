#include "../../ratatoskr/functional.hpp"
#include "../test.hpp"
#include <sstream>
#include <string>
#include <utility>

void fizz_buzz() {
  auto test_fizz_buzz = [](std::pair<int, int> range) {
    std::stringstream output;

    auto proc =
        rat::bundle{rat::thunk{}
                        .filter([](auto n) { return n % 3 == 0; })
                        .then([&output](...) { output << "Fizz"; }),
                    rat::thunk{}
                        .filter([](auto n) { return n % 5 == 0; })
                        .then([&output](...) { output << "Buzz"; }),
                    rat::thunk{}
                        .filter([](auto n) { return n % 3 != 0 && n % 5 != 0; })
                        .then([&output](auto n) { output << n; }),
                    [&output](...) { output << " "; }};

    for (int i = range.first; i < range.second; ++i) {
      proc(i);
    }

    return output.str();
  };

  test::check(test_fizz_buzz, std::pair{1, 20},
              "1 2 Fizz 4 Buzz Fizz 7 8 Fizz Buzz 11 Fizz 13 14 FizzBuzz 16 17 "
              "Fizz 19 ");
}

int main() { fizz_buzz(); }
