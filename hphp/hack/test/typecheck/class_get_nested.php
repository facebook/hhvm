<?hh // strict

final class Foo {
  public function bar(): void {
    self::self::$x;
  }
}
