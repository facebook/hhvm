<?hh
<<file:__EnableUnstableFeatures('shape_destructure')>>

function f(shape('a' => int) $x): int {
  shape('a' => $i) = $x;
  tuple($x, $y) = tuple(1, 2);
  return $i;
}
