<?hh

class D {}

abstract class C {
  abstract const type T as D;
  const type Tc = int;
  public function __construct(private this::T $x) {}
  public function bar(): this::T {
    return $this->x;
  }
}

function expect_D(D $_): void {}

function test(C $c): void {
  expect_D($c->bar());
}

function foo(C::Tc $x): C::Tc {
  return $x;
}
