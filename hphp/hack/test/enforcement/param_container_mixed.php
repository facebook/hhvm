<?hh

function f(Container<mixed> $c): void {}

function test(): void {
  $x = vec[];
  f($x);
//  ^ enforcement-at-caret
}
