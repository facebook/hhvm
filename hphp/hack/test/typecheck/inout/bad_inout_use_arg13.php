<?hh

function f(inout string $s): void {}

function test(): void {
  $x = Map { 123 => 'bar' };
  f(inout $x[123]);
}
