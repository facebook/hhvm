<?hh

trait Foo {
  public function doStuff(int $x): void {}
  public function callIt(): void {
    $this->doStuff(42);
    //             ^ hover-at-caret
  }
}
