<?hh // partial
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

class A {
  public ?int $v;
}

<<__Rx>>
function f(<<__Mutable>>A $a)[rx]: void {
  // OK
  unset($a->v);
}
