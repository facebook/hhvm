<?hh

function f(inout dynamic $d): void {}

function test(): void {
  $m = 3;
  f(inout $m);
  $m[0];
}
