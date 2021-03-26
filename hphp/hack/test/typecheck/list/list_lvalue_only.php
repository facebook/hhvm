<?hh

function foo(): void {
  list($x, list(, $y)) = tuple(1, tuple(2, 3)); // OK
  $z = list(); // not OK
}
