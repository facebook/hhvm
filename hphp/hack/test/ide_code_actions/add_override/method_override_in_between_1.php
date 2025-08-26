<?hh

class TheParent {
  public function foo(): void {}
  private function privFunc(): void {}
}

class B extends TheParent {
  public function bar(): void {}
  /*range-start*//*range-end*/
  public function baz(): void {}
}
