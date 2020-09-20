<?hh

final class Foo {
  public static function bar(int $x): void {}
}

function test(): void {
  $x = Foo::bar<>;
  $x('hello');
}
