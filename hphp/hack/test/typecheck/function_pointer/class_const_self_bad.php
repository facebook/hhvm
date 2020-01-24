<?hh

final class Foo {
  public static function baz(int $x): void {}

  public function bar(): void {
    $x = self::baz<>;
    $x('hello');
  }
}
