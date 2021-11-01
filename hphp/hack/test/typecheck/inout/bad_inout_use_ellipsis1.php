<?hh

function f(int $x, mixed ...$_): void {}

function test(): void {
  $v = 123;
  f(42, $v, inout $v);
}
