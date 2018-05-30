<?hh // strict

function foo(bool $b): int {
  $y = $b ? ($x = 42) : ($x = -1);
  return $x;
}
