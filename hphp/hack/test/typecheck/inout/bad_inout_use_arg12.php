<?hh

function f(inout int $i): void {}

function test(): void {
  $x = Vector { 3, 42 };
  f(inout $x[1]);
}
