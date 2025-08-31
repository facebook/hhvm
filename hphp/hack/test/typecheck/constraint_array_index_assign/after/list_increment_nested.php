<?hh

// Bad because writing to $x on LHS in nontrivial way and reading on RHS.
function test(varray<varray<int>> $a, varray<int> $b): int {
  $x = 0;
  list($y, $b[$x++]) = $a[$x];
  return $x;
}
