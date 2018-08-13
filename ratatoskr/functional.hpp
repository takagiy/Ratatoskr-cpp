#include <functional>
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
              std::invoke(f, x);
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

      mapping(const mapping<F, G> &) = default;
      mapping(mapping<F, G> &&) = default;

      template <class T>
      constexpr auto operator()(T &&x) -> decltype(auto) {
        return std::invoke(this->g, std::invoke(this->f, std::forward<T>(x)));
      }
    };

    template <class F>
    class mapping<F, void> : public two_function_composition<mapping<F, void>> {
    public:
      using two_function_composition<
          mapping<F, void>>::two_function_composition;

      mapping(const mapping<F, void> &) = default;
      mapping(mapping<F, void> &&) = default;

      template <class T>
      constexpr auto operator()(T &&x) {
        return std::optional{std::invoke(this->f, std::forward<T>(x))};
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

      filtering(const filtering<F, G> &) = default;
      filtering(filtering<F, G> &&) = default;

      template <class T>
      constexpr auto operator()(T &&x) -> decltype(auto) {
        return std::invoke(this->f, x)
                   ? std::invoke(this->g, std::forward<T>(x))
                   : std::nullopt;
      }
    };

    template <class F>
    class filtering<F, void>
        : public two_function_composition<filtering<F, void>> {
    public:
      using two_function_composition<
          filtering<F, void>>::two_function_composition;

      filtering(const filtering<F, void> &) = default;
      filtering(filtering<F, void> &&) = default;

      template <class T>
      constexpr auto operator()(T &&x) {
        return std::invoke(this->f, x) ? std::optional{std::forward<T>(x)}
                                       : std::nullopt;
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

      try_mapping(const try_mapping<F, G> &) = default;
      try_mapping(try_mapping<F, G> &&) = default;

      template <class T>
      constexpr auto operator()(T &&x) -> decltype(auto) {
        std::optional result = std::invoke(this->f, std::forward<T>(x));
        return result ? std::invoke(this->g, *result) : std::nullopt;
      }
    };

    template <class F>
    class try_mapping<F, void>
        : public two_function_composition<try_mapping<F, void>> {
    public:
      using two_function_composition<
          try_mapping<F, void>>::two_function_composition;

      try_mapping(const try_mapping<F, void> &) = default;
      try_mapping(try_mapping<F, void> &&) = default;

      template <class T>
      constexpr auto operator()(T &&x) {
        return std::invoke(this->f, std::forward<T>(x));
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

    thunk(const thunk<F> &) = default;
    thunk(thunk<F> &&) = default;

    template <class T>
    constexpr auto operator()(T &&x) {
      return std::invoke(f, std::forward<T>(x));
    }

    template <class G>
    constexpr auto compose(G &&g) const {
      return functional::thunk{f.compose(std::forward<G>(g))};
    }
  };

  template <>
  class thunk<void> : public detail::composable<thunk<void>> {
  public:
    thunk() = default;
    thunk(const thunk<void> &) = default;
    thunk(thunk<void> &&) = default;

    template <class T>
    constexpr auto operator()(T &&x) {
      return std::optional{std::forward<T>(x)};
    }

    template <class F>
    constexpr auto compose(F &&f) const {
      return functional::thunk{std::forward<F>(f)};
    }
  };

  thunk()->thunk<void>;

  template <class... Fs>
  class bundle {
    std::tuple<Fs...> fs;

    template <std::size_t... I, class... Ts>
    constexpr auto results_for_impl(std::index_sequence<I...>, Ts &&... xs) {
      return std::forward_as_tuple(
          std::get<I>(fs)(xs...)...,
          std::get<size - 1>(fs)(std::forward<Ts>(xs)...));
    }

    template <std::size_t... I, class... Ts>
    constexpr void apply_impl(std::index_sequence<I...>, Ts &&... xs) {
      (std::get<I>(fs)(xs...), ...);
      std::get<size - 1>(fs)(std::forward<Ts>(xs)...);
    }

    template <class F>
    static auto as_tuple_(F f) {
      return std::tuple<>{f};
    }

    template <class... Fs_>
    static auto as_tuple_(const bundle<Fs_...> &fs) {
      return fs.as_tuple();
    }
    template <class... Fs_>
    static auto as_tuple_(bundle<Fs_...> &&fs) {
      return std::move(fs).as_tuple();
    }

  public:
    static constexpr size_t size = sizeof...(Fs);

    constexpr bundle(Fs... fs) : fs(fs...) {}
    constexpr bundle(std::tuple<Fs...> fs) : fs(fs) {}

    template <class... Ts>
    [[nodiscard]] constexpr auto results_for(Ts &&... xs) {
      return results_for_impl(std::make_index_sequence<size - 1>{},
                              std::forward<Ts>(xs)...);
    }

    template <class... Ts>
    constexpr void operator()(Ts &&... xs) {
      apply_impl(std::make_index_sequence<size - 1>{}, std::forward<Ts>(xs)...);
    }

    [[nodiscard]] constexpr auto as_tuple() const & -> auto & { return fs; }
    [[nodiscard]] constexpr auto as_tuple() const && -> auto & { return fs; }
    [[nodiscard]] constexpr auto to_tuple() const { return fs; }

    template <class... Gs>
    constexpr auto bundle_with(Gs &&... gs) {
      return rat::bundle{
          std::tuple_cat(fs, as_tuple_(std::forward<Gs>(gs))...)};
    }
  };

} // namespace functional
} // namespace rat
#endif
