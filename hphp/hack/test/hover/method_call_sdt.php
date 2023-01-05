<?hh

<<__SupportDynamicType>>
class FooParent {
  public function bar(): void {}
}

<<__SupportDynamicType>>
class Foo extends FooParent {
  public function callIt(): void {
    $this->bar();
    //     ^ hover-at-caret
  }
}
