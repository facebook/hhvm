<?hh

// Bad because assigning to $x and using it on LHS.
// Lots of nesting here.
function test(varray<varray<varray<int>>> $a, varray<int> $b): int {
  $x = 0;
  list(list(list($x)), list($y, list($b[$x]))) = $a;
  return $x;
}
