#include "../ratatoskr/concurrent.hpp"
#include <chrono>
#include <iostream>
#include <thread>

int main() {
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
    catch (const close_channel &_) {
    }
  };

  std::thread producer{produce, std::move(sn)};
  std::thread consumer{consume, std::move(rc)};
  producer.join();
  consumer.join();
}
