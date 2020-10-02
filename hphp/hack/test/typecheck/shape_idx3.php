<?hh // strict

type my_shapeA = shape(?'x' => arraykey);
type my_shapeB = shape(?'x' => int);

function test(
  my_shapeA $s1,
  my_shapeB $s2,
  darray<string, arraykey> $a1,
  darray<string, int> $a2,
  arraykey $a,
  int $b,
): int {
  hh_show(idx($a1, 'x', $b));
  hh_show(idx($a2, 'x', $a));
  hh_show(Shapes::idx($s1, 'x', $b));
  hh_show(Shapes::idx($s2, 'x', $a));

  // Using "wrong" default type will be reported when trying to use the result
  // in incompatible way
  return Shapes::idx($s2, 'x', $a);
}
