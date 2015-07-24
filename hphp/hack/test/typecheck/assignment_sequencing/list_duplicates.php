<?hh // strict

// Bad because duplicate variables on LHS
function test(array<int> $a): int {
  list($x, $x, $z) = $a;
  return $x;
}
