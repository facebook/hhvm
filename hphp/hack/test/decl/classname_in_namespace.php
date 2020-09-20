<?hh

namespace {
  type A = classname<C>;
  type B = \classname<C>;

  namespace NS {
    type A = classname<C>;
    type B = \classname<C>;
  }
}
