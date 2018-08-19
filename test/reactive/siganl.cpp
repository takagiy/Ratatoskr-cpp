#include "../../ratatoskr/reactive.hpp"
#include "../test.hpp"
#include <string>
#include <thread>
#include <vector>

using namespace std::chrono_literals;

void odd_to_string() {
  auto test_odd_to_string = [](std::vector<int> input) {
    rat::scheduler scheduler;
    rat::channel<int> src;

    std::vector<std::string> output;

    rat::signal{src}
        .filter([](auto n) { return n % 2 == 1; })
        .map([](auto n) { return std::to_string(n); })
        .then([&output](auto s) { output.push_back(s); })
        .run_on(scheduler);

    for (auto n : input) {
      src.push(n);
    }
    std::this_thread::sleep_for(0.1s);

    scheduler.halt();
    scheduler.wait();

    return output;
  };

  std::vector input = {1,   2,   3,    4,   5,     6,  7, 8,  8,  9,  4,
                       2,   5,   6,    4,   6,     2,  5, 1,  5,  1,  5,
                       1,   5,   65,   4,   4,     4,  4, 5,  5,  5,  5,
                       5,   5,   31,   1,   4,     1,  4, 51, 51, 41, 11411,
                       451, 514, 5141, 141, 41341, 41, 1, 1,  41, 41, 41,
                       41,  41,  5,    4,   4,     43, 3, 1,  31, 34, 34,
                       43,  13,  31,   4,   114,   1,  1, 41, 1,  3};

  std::vector<std::string> expected = [](auto input) {
    std::vector<std::string> expected;
    for (auto n : input) {
      if (n % 2 == 1)
        expected.push_back(std::to_string(n));
    }
    return expected;
  }(input);

  test::check(test_odd_to_string, input, expected);
}

int main() { odd_to_string(); }
