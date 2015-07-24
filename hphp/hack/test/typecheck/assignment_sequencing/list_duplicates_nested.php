<?hh // strict

// Bad because reading and writing from $x on LHS
function test(array<int> $a, array<int> $b): int {
  $x = 0;
  list($x, $b[$x]) = $a;
  return $x;
}
