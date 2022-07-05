<?hh

class Foo {
  public function callIt(): void {
    $this->noSuchFunc();
    //     ^ hover-at-caret
  }
}
