#include <iostream>
#include <mutex>
#include <ratatoskr/reactive.hpp>
#include <thread>

int main() {
  using namespace std::chrono_literals;
  using std::this_thread::sleep_for;

  auto log = [](auto tag, auto data) {
    static std::mutex m{};
    std::lock_guard lock{m};
    std::cout << tag << ": " << data << " @" << std::this_thread::get_id()
              << std::endl;
  };

  rat::scheduler sch;
  rat::source<int> src1;
  rat::source<int> src2;

  src1.filter([](int n) { return n % 2 == 0; })
      .then([&log](int n) { log("rc even", n); })
      .finally([&log] { log("rc", "end"); })
      .run_on(sch, 4);

  src2.filter([](int n) { return n % 2 == 1; })
      .then([&log](int n) { log("rc odd ", n); })
      .finally([&log] { log("rc", "end"); })
      .run_on(sch, 4);

  sleep_for(1s);

  for (std::size_t i = 0; i < 100; ++i) {
    src1.push(i);
    src2.push(i);
  }

  sleep_for(1s);

  sch.halt();
  sch.wait();
}
