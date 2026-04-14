<?hh

function f(Vector<mixed> $v): void {}

function test(): void {
  $x = Vector {};
  f($x);
//  ^ enforcement-at-caret
}
