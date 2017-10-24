<?hh // strict

function f(inout array<int> $a): void {}

function test(int $x, int $y): void {
  f(inout list($x, $y));
}
