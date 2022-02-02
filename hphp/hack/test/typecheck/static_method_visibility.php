<?hh

final class Foo {
  private static function bar(): void {}
}

function baz(): void {
  Foo::bar();
}
