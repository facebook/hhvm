<?hh

class Foo {
  public static function bar(int $x): void {}
}
class Baz extends Foo {
  public static function qux(): void {
    $x = parent::bar<>;
    $x('hello');
  }
}
