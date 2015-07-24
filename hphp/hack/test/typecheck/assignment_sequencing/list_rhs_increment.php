<?hh // strict

// Bad because writing to $x on RHS
function test(array<array<int>> $a): int {
  $x = 0;
  list($x, $y) = $a[$x++];
  return $x;
}
