<?hh

final class Foo {
  public static function bar<reify T>(T $x): T {
    return $x;
  }
}

function test(): (function(int): int) {
  $x = Foo::bar<int>;

  $x(4);

  return $x;
}
