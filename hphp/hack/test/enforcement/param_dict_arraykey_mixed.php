<?hh

function f(dict<arraykey, mixed> $d): void {}

function test(): void {
  $x = dict[];
  f($x);
//  ^ enforcement-at-caret
}
