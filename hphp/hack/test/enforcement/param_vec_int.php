<?hh

function f(vec<int> $v): void {}

function test(): void {
  $x = vec[];
  f($x);
//  ^ enforcement-at-caret
}
