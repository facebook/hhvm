<?hh

function f(arraykey $x, string $y): int {
  if ($x == $y) {
    return 1;
  }
  return 0;
}
