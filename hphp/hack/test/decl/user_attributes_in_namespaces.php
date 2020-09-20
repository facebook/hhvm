<?hh

namespace X {
  class Foo implements \HH\ClassAttribute {}

  namespace Y {
    class Bar implements \HH\ClassAttribute {}
  }

  <<Foo, Y\Bar>>
  class C {}
}
