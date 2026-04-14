<?hh

function takes_int(int $x): void {}

function test(): void {
  $y = 42;
  takes_int($y);
//          ^ enforcement-at-caret
}
