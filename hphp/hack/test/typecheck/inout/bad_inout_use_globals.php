<?hh // partial

function f(inout $_): void {}

function test(): void {
  f(inout $GLOBALS);
}
