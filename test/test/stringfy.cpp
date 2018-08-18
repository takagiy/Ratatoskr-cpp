#include "../test.hpp"
#include <string>
#include <tuple>
#include <vector>

int main() {
  using namespace std::string_literals;

  auto test_stringfy = [](auto obj) { return test::stringfy(obj); };

  test::check(test_stringfy, 1, "1");
  test::check(test_stringfy, "Hello"s, "\"Hello\"");
  test::check(test_stringfy, std::vector{1, 2, 3}, "[ 1 2 3 ]");
  test::check(test_stringfy, std::tuple{1, 2, "hello"}, "{ 1 2 \"hello\" }");
}
