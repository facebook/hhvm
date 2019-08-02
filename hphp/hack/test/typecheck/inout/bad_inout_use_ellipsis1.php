<?hh // partial

function f(int $x, ...$_): void {}

function test(): void {
  $v = 123;
  f(42, $v, inout $v);
}
