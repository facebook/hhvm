<?hh

final class Foo {
  private static int $bar = 1;
}

function baz(): void {
  Foo::$bar;
}
