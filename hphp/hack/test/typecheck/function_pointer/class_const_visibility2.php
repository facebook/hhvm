<?hh

final class Foo {
  private static function bar(int $x): void {}
}

function baz(): void {
  $x = Foo::bar<>;
  $x(4);
}
