#include <optional>

namespace ratatoskr::functional {

template <class F, class G = void>
class mapping;

template <class F, class G = void>
class filtering;

template <class F = void>
class thunk;

template <class F, class G>
class mapping {
  F f;
  G g;

public:
  constexpr mapping(F f_, G g_) : f(f_), g(g_) {}

  template <class T>
  constexpr decltype(auto) operator()(T &&x) {
    return g(f(std::forward<T>(x)));
  }

  template <class H>
  constexpr auto compose(H h) {
    return functional::mapping{f, g.compose(h)};
  }

  template <class H>
  constexpr auto map(H h) {
    return this->compose(functional::mapping{h});
  }

  template <class H>
  constexpr auto filter(H h) {
    return this->compose(functional::filtering{h});
  }
};

template <class F>
class mapping<F, void> {
  F f;

public:
  constexpr mapping(F f_) : f(f_) {}

  template <class T>
  constexpr auto operator()(T &&x) {
    return std::optional{f(std::forward<T>(x))};
  }

  template <class G>
  constexpr auto compose(G g) {
    return functional::mapping{f, g};
  }

  template <class G>
  constexpr auto map(G g) {
    return this->compose(functional::mapping{g});
  }

  template <class G>
  constexpr auto filter(G g) {
    return this->compose(functional::filtering{g});
  }
};

template <class F>
mapping(F)->mapping<F, void>;

template <class F, class G>
class filtering {
  F f;
  G g;

public:
  constexpr filtering(F f_, G g_) : f(f_), g(g_) {}

  template <class T>
  constexpr decltype(auto) operator()(T &&x) {
    return f(x) ? g(std::forward<T>(x)) : std::nullopt;
  }

  template <class H>
  constexpr auto compose(H h) {
    return functional::filtering{f, g.compose(h)};
  }

  template <class H>
  constexpr auto map(H h) {
    return this->compose(functional::mapping{h});
  }

  template <class H>
  constexpr auto filter(H h) {
    return this->compose(functional::filtering{h});
  }
};

template <class F>
class filtering<F, void> {
  F f;

public:
  constexpr filtering(F f_) : f(f_) {}

  template <class T>
  constexpr auto operator()(T &&x) {
    return f(x) ? std::optional{std::forward<T>(x)} : std::nullopt;
  }

  template <class G>
  constexpr auto compose(G g) {
    return functional::filtering{f, g};
  }

  template <class G>
  constexpr auto map(G g) {
    return this->compose(functional::mapping{g});
  }

  template <class G>
  constexpr auto filter(G g) {
    return this->compose(functional::filtering{g});
  }
};

template <class F>
filtering(F)->filtering<F, void>;

template <class F>
class thunk {
  F f;

public:
  constexpr thunk(F f_) : f(f_) {}

  template <class T>
  constexpr auto operator()(T &&x) {
    return f(std::forward<T>(x));
  }

  template <class G>
  constexpr auto compose(G g) {
    return functional::thunk{f.compose(g)};
  }

  template <class G>
  constexpr auto map(G g) {
    return this->compose(functional::mapping{g});
  }

  template <class G>
  constexpr auto filter(G g) {
    return this->compose(functional::filtering{g});
  }
};

template <>
class thunk<void> {
public:
  constexpr thunk() {}

  template <class T>
  constexpr auto operator()(T &&x) {
    return f(std::forward<T>(x));
  }

  template <class F>
  constexpr auto compose(F f) {
    return functional::thunk{f};
  }

  template <class F>
  constexpr auto map(F f) {
    return this->compose(functional::mapping{f});
  }

  template <class F>
  constexpr auto filter(F f) {
    return this->compose(functional::filtering{f});
  }
};

thunk()->thunk<void>;
} // namespace ratatoskr::functional
