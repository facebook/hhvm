<?hh

function f(keyset<arraykey> $k): void {}

function test(): void {
  $x = keyset[];
  f($x);
//  ^ enforcement-at-caret
}
