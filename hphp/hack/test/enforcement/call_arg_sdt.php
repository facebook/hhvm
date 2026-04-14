<?hh

function takes_int(int $x): int {
  return $x;
}

function test(): void {
  $y = 42;
  takes_int($y);
//          ^ enforcement-at-caret
}
