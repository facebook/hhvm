<?hh // strict
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

class A {
  <<__Rx>>
  public function __construct(public int $x) {}
  <<__Rx, __Mutable>>
  public function g(): int {
    return 42;
  }
}

<<__Rx>>
function f(<<__MaybeMutable>>A $a): int {
  // ERROR to call mutable method
  return $a->g();
}
