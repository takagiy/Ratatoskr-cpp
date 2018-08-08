#include <optional>
#include <utility>

#ifndef RATATOSKR_FUNCTIONAL_HPP
#define RATATOSKR_FUNCTIONAL_HPP

namespace rat {
inline namespace functional {

  namespace detail {

    template <class T>
    class composable;

    template <class T>
    class two_function_composition;

    template <class F, class G = void>
    class mapping;

    template <class F, class G = void>
    class filtering;

    template <class F, class G = void>
    class try_mapping;

    template <class T>
    class composable {
    public:
      template <class F>
      constexpr auto map(F &&f) const {
        return static_cast<const T *>(this)->compose(
            mapping{std::forward<F>(f)});
      };

      template <class F>
      constexpr auto filter(F &&f) const {
        return static_cast<const T *>(this)->compose(
            filtering{std::forward<F>(f)});
      }

      template <class F>
      constexpr auto then(F &&f) const {
        return static_cast<const T *>(this)->compose(
            mapping{[f = std::forward<F>(f)](auto &&x) {
              f(x);
              return x;
            }});
      }

      template <class F>
      constexpr auto try_map(F &&f) const {
        return static_cast<const T *>(this)->compose(
            try_mapping{std::forward<F>(f)});
      }
    };

    template <template <class, class> class Composable, class F, class G>
    class two_function_composition<Composable<F, G>>
        : public composable<two_function_composition<Composable<F, G>>> {
    protected:
      F f;
      G g;

    public:
      constexpr two_function_composition(const F &f_, const G &g_)
          : f(f_), g(g_) {}
      constexpr two_function_composition(const F &f_, G &&g_)
          : f(f_), g(std::move(g_)) {}
      constexpr two_function_composition(F &&f_, G &&g_)
          : f(std::move(f_)), g(std::move(g_)) {}
      constexpr two_function_composition(F &&f_, const G &g_)
          : f(std::move(f_)), g(g_) {}

      template <class H>
      constexpr auto compose(H &&h) const {
        return Composable{f, g.compose(std::forward<H>(h))};
      }
    };

    template <template <class, class> class Composable, class F>
    class two_function_composition<Composable<F, void>>
        : public composable<two_function_composition<Composable<F, void>>> {
    protected:
      F f;

    public:
      constexpr two_function_composition(const F &f_) : f(f_) {}
      constexpr two_function_composition(F &&f_) : f(std::move(f_)) {}

      template <class G>
      constexpr auto compose(G &&g) const {
        return Composable{f, std::forward<G>(g)};
      }
    };

    template <class F, class G>
    class mapping : public two_function_composition<mapping<F, G>> {
    public:
      using two_function_composition<mapping<F, G>>::two_function_composition;

      template <class T>
      constexpr decltype(auto) operator()(T &&x) {
        return this->g(this->f(std::forward<T>(x)));
      }
    };

    template <class F>
    class mapping<F, void> : public two_function_composition<mapping<F, void>> {
    public:
      using two_function_composition<
          mapping<F, void>>::two_function_composition;

      template <class T>
      constexpr auto operator()(T &&x) {
        return std::optional{this->f(std::forward<T>(x))};
      }
    };

    template <class F, class G>
    mapping(F, G)->mapping<F, G>;

    template <class F>
    mapping(F)->mapping<F, void>;

    template <class F, class G>
    class filtering : public two_function_composition<filtering<F, G>> {
    public:
      using two_function_composition<filtering<F, G>>::two_function_composition;

      template <class T>
      constexpr decltype(auto) operator()(T &&x) {
        return this->f(x) ? this->g(std::forward<T>(x)) : std::nullopt;
      }
    };

    template <class F>
    class filtering<F, void>
        : public two_function_composition<filtering<F, void>> {
    public:
      using two_function_composition<
          filtering<F, void>>::two_function_composition;

      template <class T>
      constexpr auto operator()(T &&x) {
        return this->f(x) ? std::optional{std::forward<T>(x)} : std::nullopt;
      }
    };

    template <class F, class G>
    filtering(F, G)->filtering<F, G>;

    template <class F>
    filtering(F)->filtering<F, void>;

    template <class F, class G>
    class try_mapping : public two_function_composition<try_mapping<F, G>> {
    public:
      using two_function_composition<
          try_mapping<F, G>>::two_function_composition;

      template <class T>
      constexpr decltype(auto) operator()(T &&x) {
        std::optional result = this->f(std::forward<T>(x));
        return result ? this->g(*result) : std::nullopt;
      }
    };

    template <class F>
    class try_mapping<F, void>
        : public two_function_composition<try_mapping<F, void>> {
    public:
      using two_function_composition<
          try_mapping<F, void>>::two_function_composition;

      template <class T>
      constexpr auto operator()(T &&x) {
        return this->f(std::forward<T>(x));
      }
    };

    template <class F, class G>
    try_mapping(F, G)->try_mapping<F, G>;

    template <class F>
    try_mapping(F)->try_mapping<F, void>;
  }; // namespace detail

  template <class F = void>
  class thunk;

  template <class F>
  class thunk : public detail::composable<thunk<F>> {
    F f;

  public:
    constexpr thunk(const F &f_) : f(f_) {}
    constexpr thunk(F &&f_) : f(std::move(f_)) {}

    template <class T>
    constexpr auto operator()(T &&x) {
      return f(std::forward<T>(x));
    }

    template <class G>
    constexpr auto compose(G &&g) const {
      return functional::thunk{f.compose(std::forward<G>(g))};
    }
  };

  template <>
  class thunk<void> : public detail::composable<thunk<void>> {
  public:
    constexpr thunk() {}

    template <class T>
    constexpr auto operator()(T &&x) {
      return f(std::forward<T>(x));
    }

    template <class F>
    constexpr auto compose(F &&f) const {
      return functional::thunk{std::forward<F>(f)};
    }
  };

  thunk()->thunk<void>;

} // namespace functional
} // namespace rat
#endif
