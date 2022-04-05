<?hh

class Foo {
  public function __construct(int $x): void {}

  public static function newInstance(): void {
    new self(42);
    //       ^ hover-at-caret
  }
}
