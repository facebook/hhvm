<?hh

function takes_int(int $x): void {}
//                 ^ enforcement-at-caret

function takes_generic<T>(T $x): void {}

function returns_int(): int {
//                      ^ enforcement-at-caret
  $y = 42;
  takes_int($y);
//          ^ enforcement-at-caret
  takes_generic($y);
//              ^ enforcement-at-caret
  return $y;
//       ^ enforcement-at-caret
}
