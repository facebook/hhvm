<?hh

function f(KeyedContainer<arraykey, mixed> $c): void {}

function test(): void {
  $x = dict[];
  f($x);
//  ^ enforcement-at-caret
}
