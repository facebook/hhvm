<?hh // partial
class A {
  // ERROR
  <<__Rx>>
  public function f(<<__Mutable, __OwnedMutable>> $x): void {
  }
}
