<?hh

namespace A\B {
  class X implements \HH\FunctionAttribute {
    public function __construct(public int $i) {}
  }

  class Y implements \HH\FunctionAttribute {
    public function __construct(public string $s) {}
  }
}

namespace {
  use A\B\Y;

  // built in attributes not elaborated
  <<__Memoize>>
  function f(): void {}

  <<A\B\X(3)>>
  function g(): void {}

  <<Y(4)>>
  function h(): void {}
}
