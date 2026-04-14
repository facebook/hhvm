<?hh

function f(Map<arraykey, mixed> $m): void {}

function test(): void {
  $x = Map {};
  f($x);
//  ^ enforcement-at-caret
}
