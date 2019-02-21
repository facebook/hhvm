<?hh // partial

class A {
  // ERROR
  public function f(<<__OwnedMutable>> A $a) {
  }
}
