<?hh

// Bad because reading and writing from $x on LHS
function test(varray<int> $a, varray<int> $b): int {
  $x = 0;
  list($x, $b[$x]) = $a;
  return $x;
}
