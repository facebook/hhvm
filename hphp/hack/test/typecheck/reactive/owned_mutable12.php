<?hh

class A {
  <<__Rx>>
  public function f(<<__Mutable>> A $a): void {
  }
}

class B extends A {
  // ERROR - cannot treat mutable as owned mutable
  <<__Rx>>
  public function f(<<__OwnedMutable>> A $a): void {
  }
}
