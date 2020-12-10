<?hh // partial
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

class A {
  public ?int $v;
}

<<__Rx>>
function f(<<__Mutable>>A $a): void {
  // OK
  unset($a->v);
}
