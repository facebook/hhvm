<?hh

class Foo {
  public function __construct(int $x): void {}
}

class FooChild extends Foo {
  public function __construct(int $y): void {}

  public static function newInstance(): void {
    new parent(42);
    //         ^ hover-at-caret
  }
}
