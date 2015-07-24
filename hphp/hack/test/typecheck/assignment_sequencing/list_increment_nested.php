<?hh // strict

// Bad because writing to $x on LHS in nontrivial way and reading on RHS.
function test(array<array<int>> $a, array<int> $b): int {
  $x = 0;
  list($y, $b[$x++]) = $a[$x];
  return $x;
}
