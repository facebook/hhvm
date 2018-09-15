<?hh // strict

function f(inout string $s): void {}

function test(): void {
  $x = Pair { 'herp', 'derp' };
  f(inout $x[1]);
}
