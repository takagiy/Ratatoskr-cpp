#include "../ratatoskr/trvial.hpp"
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <tuple>
#include <utility>

namespace test {
namespace detail {
  template <class T>
  constexpr auto check_show(std::nullptr_t)
      -> decltype(std::cout << std::declval<T>(), bool{}) {
    return true;
  }
  template <class T>
  constexpr auto check_show(...) {
    return false;
  }

  template <class T>
  constexpr auto show() {
    return check_show<T>(nullptr);
  }

  template <class T>
  constexpr auto check_iterate(std::nullptr_t)
      -> decltype(std::declval<T>().begin() == std::declval<T>().end(),
                  bool{}) {
    return true;
  }
  template <class T>
  constexpr auto check_iterate(...) {
    return false;
  }

  template <class T>
  constexpr auto iterate() {
    return check_iterate<T>(nullptr);
  }
} // namespace detail

template <class T>
auto stringfy(T &&object) -> std::string {

  std::stringstream ss;

  if constexpr (detail::show<T &&>()) {
    ss << object;
  }
  else if constexpr (detail::iterate<T &&>()) {
    ss << "[ ";
    for (auto &&element : object) {
      ss << stringfy(element) << " ";
    }
    ss << "]";
  }

  return ss.str();
}

auto stringfy(const std::string string) -> std::string {
  return "\"" + string + "\"";
}

auto stringfy(const char *c_string) -> std::string {
  std::stringstream ss;
  ss << "\"" << c_string << "\"";
  return ss.str();
}

template <class... Ts>
auto stringfy(const std::tuple<Ts...> tuple) -> std::string {
  std::stringstream ss;
  ss << "{ ";
  rat::trivial::tuples::for_each(
      tuple, [&ss](auto &&element) { ss << stringfy(element) << " "; });
  ss << "}";
  return ss.str();
}

template <class Function, class Input>
void check(Function test, Input input,
           std::invoke_result_t<Function, Input> expected) {

  std::cout << "Test start." << std::endl;

  auto input_str = stringfy(input);
  auto expected_str = stringfy(expected);
  auto output_str = stringfy(test(input));

  if (output_str != expected_str) {
    std::cout << "-> Failure.\n";
    std::cout << "  Input:    " << input_str << "\n";
    std::cout << "  Expected: " << expected_str << "\n";
    std::cout << "  Output:   " << output_str << std::endl;
    return;
  }

  std::cout << "-> Success." << std::endl;
}
} // namespace test
