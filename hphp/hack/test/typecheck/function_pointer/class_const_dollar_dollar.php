<?hh

final class Foo {
  public static function bar(int $x): void {}
}

/* TODO: Should error */
function baz(): void {
  $x = Foo::class |> $$::bar<>;
  $x(4);
}
