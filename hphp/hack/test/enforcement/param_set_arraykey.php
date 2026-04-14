<?hh

function f(Set<arraykey> $s): void {}

function test(): void {
  $x = Set {};
  f($x);
//  ^ enforcement-at-caret
}
