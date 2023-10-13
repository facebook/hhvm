<?hh

function foo(bool $b): int {
  // broken for now because eif is broken
  $y = $b ? ($x = 42) : ($x = -1);
  return $x;
}
