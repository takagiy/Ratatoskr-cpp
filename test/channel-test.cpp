#include "../ratatoskr/concurrent.hpp"
#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>

int main() {
  using namespace ratatoskr::concurrent;
  using namespace std::chrono_literals;

  auto [sn, rc] = make_channel<int>();

  std::mutex io_mutex;

  auto produce = [&io_mutex](auto sn) {
    for (int i = 0; i < 10; ++i) {
      {
        std::lock_guard lock{io_mutex};
        std::cout << "send: " << i << std::endl;
      }
      sn.push(i);
      std::this_thread::sleep_for(1s);
    }
    {
      std::lock_guard lock{io_mutex};
      std::cout << "send: close" << std::endl;
    }
    sn.close();
  };

  auto consume = [&io_mutex](auto rc) {
    try {
      while (true) {
        auto &&x = rc.next();
        std::lock_guard lock{io_mutex};
        std::cout << "recieve: " << x << std::endl;
      }
    }
    catch (const close_channel &) {
      std::lock_guard lock{io_mutex};
      std::cout << "receive: close" << std::endl;
    }
  };

  std::thread producer{produce, std::move(sn)};
  std::thread consumer{consume, std::move(rc)};
  producer.join();
  consumer.join();
}
