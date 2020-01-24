<?hh

final class Foo {
  public static function bar(int $x): void {}
}

function baz(): void {
  $x = Foo::class |> $$::bar<>;
  $x('hello');
}
