<?hh // partial

class A {
  <<__Rx, __Mutable>>
  public function m(): void {
  }
}

<<__Rx>>
function f(Rx<(function(): OwnedMutable<A>)> $f): void {
  $a = \HH\Rx\mutable($f());
  $a->m();
}
