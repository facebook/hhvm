<?hh // partial

class A {
  public ?int $v;
}

<<__Rx>>
function f(A $a): void {
  // ERROR
  unset($a->v);
}
