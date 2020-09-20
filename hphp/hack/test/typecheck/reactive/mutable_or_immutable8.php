<?hh // strict

class A {
  <<__Rx>>
  public function __construct(public int $x) {}
}

<<__Rx>>
function f(<<__MaybeMutable>>A $a): int {
  return $a->x;
}

<<__Rx>>
function g(): void {
  $a = \HH\Rx\mutable(new A(10));
  // OK to pass mutable as maybe mutable
  $v1 = f($a);
  $a1 = \HH\Rx\freeze($a);
  // OK to pass immutable as maybe mutable
  $v2 = f($a1);
}
