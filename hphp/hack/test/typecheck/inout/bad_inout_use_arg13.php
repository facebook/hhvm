<?hh // strict

function f(inout string $s): void {}

function test(): void {
  $x = ImmMap { 123 => 'bar' };
  f(inout $x[123]);
}
