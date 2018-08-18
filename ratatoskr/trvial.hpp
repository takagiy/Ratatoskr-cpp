#include <cstddef>
#include <functional>
#include <tuple>
#include <type_traits>
#include <utility>

#ifndef RATATOSKR_TRIVIAL_HPP
#define RATATOSKR_TRIVIAL_HPP

namespace rat {
inline namespace trivial {

  namespace detail {
    template <class Tuple, class F, size_t... I>
    void for_each_impl(Tuple &&t, F f, std::index_sequence<I...>) {
      (std::invoke(f, std::get<I>(std::forward<Tuple>(t))), ...);
    }

    template <class Tuple, class F, size_t... I>
    auto transform_impl(Tuple &&t, F f, std::index_sequence<I...>) {
      return std::make_tuple(
          std::invoke(f, std::get<I>(std::forward<Tuple>(t)))...);
    }
  } // namespace detail

  namespace tuples {

    template <class Tuple, class F>
    void for_each(Tuple &&t, F f) {
      detail::for_each_impl(
          std::forward<Tuple>(t), f,
          std::make_index_sequence<
              std::tuple_size_v<std::remove_reference_t<Tuple>>>{});
    }

    template <class Tuple, class F>
    auto transform(Tuple &&t, F f) {
      return detail::transform_impl(
          std::forward<Tuple>(t), f,
          std::make_index_sequence<
              std::tuple_size_v<std::remove_reference_t<Tuple>>>{});
    }
  }; // namespace tuples

} // namespace trivial
} // namespace rat

#endif
