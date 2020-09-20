<?hh

final class Foo {
  private static function bar(int $x): void {}

  public function baz(): void {
    $x = self::bar<>;
    $x(4);
  }
}
