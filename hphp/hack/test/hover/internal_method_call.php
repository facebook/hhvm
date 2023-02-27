internal class A {
  internal function bar(): void {}
  internal function baz(): void {
    $this->bar();
    //     ^ hover-at-caret
  }
}
