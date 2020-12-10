<?hh
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

class A {
  <<__Rx>>
  public function f(<<__OwnedMutable>> A $a): void {
  }
}

class B extends A {
  // ERROR: cannot treat mutably owned value as immutable
  <<__Rx>>
  public function f(A $a): void {
  }
}
