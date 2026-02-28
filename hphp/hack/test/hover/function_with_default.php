<?hh

function add_one(int $x = 0): int {
  return $x + 1;
}

function call_it(): void {
  $y = add_one(42);
  //   ^ hover-at-caret
}
