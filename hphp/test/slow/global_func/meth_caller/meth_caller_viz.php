<?hh

namespace {
class A {
  protected function f($x) { return "A::f($x)"; }
  private function g($x) { return "A::g($x)"; }
}
}

namespace Q {
class B extends \A {
  protected function f($x) { return "Q\\B::f($x)"; }
  private function g($x) { return "Q\\B::g($x)"; }

  function doit() {
    \var_dump(\HH\meth_caller(\A::class, "f")(new X\C(), 2));
    \var_dump(\HH\meth_caller(\A::class, "f")(new B(), 2));
    \var_dump(\HH\meth_caller( B::class, "f")(new \Q\B(), 2));
    \var_dump(\HH\meth_caller(\A::class, "f")(new \A(), 2));

    \var_dump(\HH\meth_caller(\Q\X\C::class, "g")(new \Q\X\C(), 2));
    \var_dump(\HH\meth_caller(  \Q\B::class, "g")(new B(), 2));
    \var_dump(\HH\meth_caller(  \Q\B::class, "g")(new \Q\B(), 2));
    \var_dump(\HH\meth_caller(    \A::class, "g")(new \A(), 2));
  }
}
}

namespace Q\X {
class C extends \Q\B {
  protected function f($x) { return "Q\\X\\C::f($x)"; }
  private function g($x) { return "Q\\X\\C::g($x)"; }
}
}

namespace {
<<__EntryPoint>>
function main(): void {
  new Q\B()->doit();
}
}
