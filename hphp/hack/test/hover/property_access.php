<?hh

class Foo {
  public int $x = 1;

  public function demo(): void {
    $this->x;
    //     ^ hover-at-caret
  }
}
