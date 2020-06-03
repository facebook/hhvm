<?hh

namespace {
  type A = Traversable<int>;
  type B = \Traversable<int>;

  namespace NS {
    type A = Traversable<int>;
    type B = \Traversable<int>;
  }
}
