#include "../ratatoskr/reactive.hpp"
#include <chrono>
#include <cstddef>
#include <iostream>
#include <mutex>
#include <thread>

auto main() -> int {
  using namespace std::chrono_literals;
  auto log = [](auto tag, auto data) {
    static std::mutex m;

    std::lock_guard lock{m};
    std::cout << tag << ": " << data << " @" << std::this_thread::get_id()
              << std::endl;
  };

  rat::scheduler sch;
  rat::channel<int> odd;
  rat::channel<int> even;

  rat::signal{odd}
      .filter([](int n) { return n % 2 == 1; })
      .then([&log](int n) { log("receive(odd) ", n); })
      .finally([&log] { log("receive(odd) ", "close"); })
      .run_on(sch, 4);

  rat::signal{even}
      .filter([](int n) { return n % 2 == 0; })
      .then([&log](int n) { log("receive(even)", n); })
      .finally([&log] { log("receive(even)", "close"); })
      .run_on(sch, 4);

  std::this_thread::sleep_for(1s);

  for (std::size_t i = 0; i < 20; ++i) {
    odd.push(i);
    even.push(i);
  }

  std::this_thread::sleep_for(1s);

  sch.halt();
  sch.wait();
}
