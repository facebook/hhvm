<?hh

class Foo {
  public supportdyn<mixed> $x = 1;

  public function demo(): void {
    $this->x;
    //     ^ hover-at-caret
  }
}
