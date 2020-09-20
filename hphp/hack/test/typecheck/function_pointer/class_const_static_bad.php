<?hh

class Foo {
  public static function bar(int $x): void {}

  public static function baz(): void {
    $x = static::bar<>;
    $x('hello');
  }
}
