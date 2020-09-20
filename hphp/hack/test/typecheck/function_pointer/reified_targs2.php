<?hh

function foo<reify T, reify T2>(T $x, T2 $y): void {}

function test(): (function(int, string): void) {
  $x = foo<int, string>;

  $x(4, "Hello");

  return $x;
}
