<?hh // strict

// Bad because assigning to $x and using it on LHS.
// Lots of nesting here.
function test(array<array<array<int>>> $a, array<int> $b): int {
  $x = 0;
  list(list(list($x)), list($y, list($b[$x]))) = $a;
  return $x;
}
