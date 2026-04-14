<?hh

function f(dict<string, int> $d): void {}

function test(): void {
  $x = dict[];
  f($x);
//  ^ enforcement-at-caret
}
