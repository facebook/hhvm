<?hh // strict

class Foo {
  const type T = int;
  public static function f(self::T $x): int {
    return $x + 1;
  }
}

function test(): void {
  var_dump(Foo::f(10));
}
