<?hh // strict

// Bad because writing to $x twice on LHS
function test(array<int> $a, array<int> $b): int {
  $x = 0;
  list($b[++$x], $b[++$x]) = $a;
  return $x;
}
