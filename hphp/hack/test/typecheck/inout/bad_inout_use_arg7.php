<?hh // strict

function f(inout int $i): void {}

class C {
  public static int $x = 42;
}

function test(): void {
  f(inout C::$x);
}
