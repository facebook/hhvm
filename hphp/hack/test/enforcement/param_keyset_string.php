<?hh

function f(keyset<string> $k): void {}

function test(): void {
  $x = keyset[];
  f($x);
//  ^ enforcement-at-caret
}
