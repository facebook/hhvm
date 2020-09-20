<?hh

final class Foo {
  public static function bar<reify T, reify T2>(T $x, T2 $y): void {}
}

function test(): (function(int, string): void) {
  $x = Foo::bar<int, string>;

  $x(4, "Hello");

  return $x;
}
