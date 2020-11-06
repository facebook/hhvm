<?hh // strict


namespace {

  namespace NS {
    class C {}
  }
  namespace C {
    class X {}
  }

  use type NS\C;

  class D extends C {}

  function f(C\X $x): void {}
}
