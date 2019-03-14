<?hh // partial

class A {
  <<__Rx>>
  public function f(<<__OwnedMutable>> A $a) {
  }
}

class B extends A {
  // OK
  <<__Rx>>
  public function f(<<__MaybeMutable>> A $a) {
  }
}
