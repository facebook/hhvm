<?hh

class FooParent {
  public function bar(): void {}
}

class Foo extends FooParent {
  public function callIt(): void {
    $this->bar();
    //     ^ hover-at-caret
  }
}
