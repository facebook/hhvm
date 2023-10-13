<?hh

function foo(bool $b): int {
  $y = $b ? ($x = 42) : -1;
  return $x;
}
