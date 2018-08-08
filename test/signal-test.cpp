#include <iostream>
#include <mutex>
#include <ratatoskr/reactive.hpp>
#include <thread>

int main() {
  using namespace std::chrono_literals;
  using std::this_thread::sleep_for;

  auto log = [](auto tag, auto data) {
    static std::mutex m;
    std::lock_guard lock{m};
    std::cout << tag << ": " << data << " @" << std::this_thread::get_id()
              << std::endl;
  };

  rat::scheduler sch;
  rat::source<int> odd;
  rat::source<int> even;

  odd.filter([](int n) { return n % 2 == 1; })
      .then([&log](int n) { log("odd ", n); })
      .finally([&log] { log("odd ", "end"); })
      .run_on(sch, 4);

  even.filter([](int n) { return n % 2 == 0; })
      .then([&log](int n) { log("even", n); })
      .finally([&log] { log("even", "end"); })
      .run_on(sch, 4);

  sleep_for(1s);

  for (std::size_t i = 0; i < 100; ++i) {
    odd.push(i);
    even.push(i);
  }

  sleep_for(1s);

  sch.halt();
  sch.wait();
}
