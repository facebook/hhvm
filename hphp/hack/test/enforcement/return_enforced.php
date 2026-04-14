<?hh

function returns_int(): int {
  $x = 42;
  return $x;
//       ^ enforcement-at-caret
}
