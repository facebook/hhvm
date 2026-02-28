<?hh

// Bad because duplicate variables on LHS
function test(varray<int> $a): int {
  list($x, $x, $z) = $a;
  return $x;
}
