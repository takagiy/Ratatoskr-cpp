#include <optional>
#include <utility>

#ifndef RATATOSKR_FUNCTIONAL_HPP
#define RATATOSKR_FUNCTIONAL_HPP

namespace rat {
inline namespace functional {

template <class T>
class map_and_filterable;

template <class T>
class composable;

template <class F, class G = void>
class mapping;

template <class F, class G = void>
class filtering;

template <class F = void>
class thunk;

template <class T>
class map_and_filterable {
public:
  template <class F>
  constexpr auto map(F &&f) const {
    return static_cast<const T *>(this)->compose(mapping{std::forward<F>(f)});
  };

  template <class F>
  constexpr auto filter(F &&f) const {
    return static_cast<const T *>(this)->compose(filtering{std::forward<F>(f)});
  }
};

template <template <class, class> class Composable, class F, class G>
class composable<Composable<F, G>>
    : public map_and_filterable<composable<Composable<F, G>>> {
protected:
  F f;
  G g;

public:
  constexpr composable(const F &f_, const G &g_) : f(f_), g(g_) {}
  constexpr composable(const F &f_, G &&g_) : f(f_), g(std::move(g_)) {}
  constexpr composable(F &&f_, G &&g_) : f(std::move(f_)), g(std::move(g_)) {}
  constexpr composable(F &&f_, const G &g_) : f(std::move(f_)), g(g_) {}

  template <class H>
  constexpr auto compose(H &&h) const {
    return Composable{f, g.compose(std::forward<H>(h))};
  }
};

template <template <class, class> class Composable, class F>
class composable<Composable<F, void>>
    : public map_and_filterable<composable<Composable<F, void>>> {
protected:
  F f;

public:
  constexpr composable(const F &f_) : f(f_) {}
  constexpr composable(F &&f_) : f(std::move(f_)) {}

  template <class G>
  constexpr auto compose(G &&g) const {
    return Composable{f, std::forward<G>(g)};
  }
};

template <class F, class G>
class mapping : public composable<mapping<F, G>> {
public:
  using composable<mapping<F, G>>::composable;

  template <class T>
  constexpr decltype(auto) operator()(T &&x) {
    return this->g(this->f(std::forward<T>(x)));
  }
};

template <class F>
class mapping<F, void> : public composable<mapping<F, void>> {
public:
  using composable<mapping<F, void>>::composable;

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
class filtering : public composable<filtering<F, G>> {
public:
  using composable<filtering<F, G>>::composable;

  template <class T>
  constexpr decltype(auto) operator()(T &&x) {
    return this->f(x) ? this->g(std::forward<T>(x)) : std::nullopt;
  }
};

template <class F>
class filtering<F, void> : public composable<filtering<F, void>> {
public:
  using composable<filtering<F, void>>::composable;

  template <class T>
  constexpr auto operator()(T &&x) {
    return this->f(x) ? std::optional{std::forward<T>(x)} : std::nullopt;
  }
};

template <class F, class G>
filtering(F, G)->filtering<F, G>;

template <class F>
filtering(F)->filtering<F, void>;

template <class F>
class thunk : public map_and_filterable<thunk<F>> {
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
class thunk<void> : public map_and_filterable<thunk<void>> {
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
