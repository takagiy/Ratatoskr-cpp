#include <array>
#include <iostream>
#include <thread>

std::array<std::thread, 3> foo() {
  std::array<std::thread, 3> a;
  return a;
}

int main() {
  auto a = foo();
  a = foo();
  /* for (auto &&i : a) {
     std::cout << i << std::endl;
   }*/
}
