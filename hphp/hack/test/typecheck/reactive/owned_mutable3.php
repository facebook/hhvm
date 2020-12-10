<?hh
<<file: __EnableUnstableFeatures('coeffects_provisional')>>
class A {
  // ERROR
  <<__Rx>>
  public function f(<<__Mutable, __OwnedMutable>> mixed $x): void {
  }
}
