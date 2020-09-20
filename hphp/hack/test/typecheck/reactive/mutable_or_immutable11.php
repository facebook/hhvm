<?hh // strict

class A {
  <<__Rx>>
  public function __construct(public int $x) {}
  <<__Rx>>
  public function g(): int {
    return 42;
  }
}

<<__Rx>>
function f(<<__MaybeMutable>>A $a): int {
  // ERROR to call immutable method
  return $a->g();
}
