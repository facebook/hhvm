<?hh

abstract class A {
  const type T as arraykey = arraykey;
}

trait T {
  const type T = int;
}

final class B extends A {
  use T;
  public static function cast(mixed $x): int {
    return $x is this::T ? $x : 0;
  }
}

<<__EntryPoint>>
function main(): void {
  B::cast('break it'); // Error, string expected int
  echo "No type hint violation";
}
