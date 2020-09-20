<?hh

final class Foo {
  public static function bar(int $x): void {}
}

final class Qux {
}

function baz(): void {
  $x = Qux::class |> $$::bar<>;
  $x('hello');
}
