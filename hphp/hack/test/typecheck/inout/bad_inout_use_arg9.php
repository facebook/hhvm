<?hh // strict

function f(inout string $s): void {}

class C {
  public static dict<int, string> $x = dict[42 => 'foo'];
}

function test(): void {
  f(inout C::$x[42]);
}
