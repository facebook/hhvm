<?hh

// Bad because writing to $x on RHS
function test(varray<varray<int>> $a): int {
  $x = 0;
  list($x, $y) = $a[$x++];
  return $x;
}
