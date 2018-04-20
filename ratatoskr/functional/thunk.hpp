#include <optional>

namespace ratatoskr::functional {
template <class F, class G = void>
class mapping;

template <class F, class G = void>
class filtering;

template <class F, class G>
class mapping {
  F f;
  G g;

public:
  mapping(F f_, G g_) : f(f_), g(g_) {}

  template <class T>
  auto operator()(T x) {
    return g(f(x));
  }

  template <class H>
  auto compose(H h) {
    return functional::mapping{f, g.compose(h)};
  }

  template <class H>
  auto map(H h) {
    return this->compose(functional::mapping{h});
  }

  template <class H>
  auto filter(H h) {
    return this->compose(functional::filtering{h});
  }
};

template <class F>
class mapping<F> {
  F f;

public:
  mapping(F f_) : f(f_) {}

  template <class T>
  auto operator()(T x) {
    return std::optional{f(x)};
  }

  template <class G>
  auto compose(G g) {
    return functional::mapping{f, g};
  }

  template <class G>
  auto map(G g) {
    return this->compose(functional::mapping{g});
  }

  template <class G>
  auto filter(G g) {
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
  filtering(F f_, G g_) : f(f_), g(g_) {}

  template <class T>
  auto operator()(T x) {
    return f(x) ? g(x) : std::nullopt;
  }

  template <class H>
  auto compose(H h) {
    return functional::filtering{f, g.compose(h)};
  }
};

template <class F>
class filtering<F> {
  F f;

public:
  filtering(F f_) : f(f_) {}

  template <class T>
  std::optional<T> operator()(T x) {
    return f(x) ? std::optional{x} : std::nullopt;
  }

  template <class G>
  auto compose(G g) {
    return functional::filtering{f, g};
  }

  template <class H>
  auto map(H h) {
    return this->compose(functional::mapping{h});
  }

  template <class H>
  auto filter(H h) {
    return this->compose(functional::filtering{h});
  }
};

template <class F>
filtering(F)->filtering<F, void>;
} // namespace ratatoskr::functional
