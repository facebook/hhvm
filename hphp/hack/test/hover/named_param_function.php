<?hh

function add_one(int $x, float $y): int {
  return $x + 1;
}

function call_it(): void {
  $y = add_one(42, 1.0);
  //               ^ hover-at-caret
}
