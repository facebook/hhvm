<?hh

function f(vec<mixed> $v): void {}

function test(): void {
  $x = vec[];
  f($x);
//  ^ enforcement-at-caret
}
