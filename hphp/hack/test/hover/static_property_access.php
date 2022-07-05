<?hh

class Foo {
  public static int $x = 1;

  public function demo(): void {
    $this::$x;
    //     ^ hover-at-caret
  }
}
