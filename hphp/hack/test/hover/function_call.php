<?hh

/**
 * Adds one to the argument.
 */
function add_one(int $x): int {
  return $x + 1;
}

function call_it(): void {
  $y = add_one(42);
  //   ^ hover-at-caret
}
