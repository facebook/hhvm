<?hh
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

class A {
  <<__Rx, __Mutable>>
  public function m(): void {
  }
}

<<__Rx>>
function f(Rx<(function(): Mutable<A>)> $f): void {
  $a = \HH\Rx\mutable($f());
  $a->m();
}
