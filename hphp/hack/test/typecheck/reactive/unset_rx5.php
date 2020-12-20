<?hh
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

class A {
  public ?int $v;
}

<<__Rx>>
function f(dict<int, dict<int, A>> $a)[rx]: void {
  // OK
  unset($a[0][1]);
}
