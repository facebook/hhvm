<?hh // partial

class A {
  <<__Rx>>
  public function f(<<__OwnedMutable>> A $a) {
  }
}

class B extends A {
  // OK to treat owned as mutable: $a in B::f can be changed but cannot be saved
  <<__Rx>>
  public function f(<<__Mutable>> A $a) {
  }
}
