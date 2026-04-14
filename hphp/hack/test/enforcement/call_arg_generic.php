<?hh

function takes_generic<T>(T $x): void {}

function test(): void {
  $y = 42;
  takes_generic($y);
//              ^ enforcement-at-caret
}
