<?hh // partial

class A {
  <<__Rx>>
  public function f(<<__OwnedMutable>> A $a) {
  }
}

class B extends A {
  // ERROR: cannot treat mutably owned value as immutable
  <<__Rx>>
  public function f(A $a) {
  }
}
