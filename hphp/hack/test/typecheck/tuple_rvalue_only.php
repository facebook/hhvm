<?hh

function foo(): void {
  $x = tuple(1, "foo"); // OK
  tuple($a, $b) = vec[1, 2]; // not OK
}
