<?hh

function foo(varray<varray<int>> $x): int {
  $y = 0;
  return $x[$y][$y++];
}
