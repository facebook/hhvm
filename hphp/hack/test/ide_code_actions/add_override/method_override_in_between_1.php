<?hh

class TheParent {
  public function foo(): void {}
}

class B extends TheParent {
  public function bar(): void {}
  /*range-start*//*range-end*/
  public function baz(): void {}
}
