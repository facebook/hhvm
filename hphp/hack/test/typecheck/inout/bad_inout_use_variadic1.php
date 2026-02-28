<?hh

function f(string $x, int ...$ys): void {}

function test(): void {
  $v = 123;
  f('bar', $v, inout $v);
}
