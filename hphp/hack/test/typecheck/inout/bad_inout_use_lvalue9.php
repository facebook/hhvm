<?hh

function f(inout varray<int> $a): void {}

function test(int $x, int $y): void {
  f(inout tuple($x, $y));
}
