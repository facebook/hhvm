<?hh

function f(dict<string, mixed> $d): void {}

function test(): void {
  $x = dict[];
  f($x);
//  ^ enforcement-at-caret
}
