<?hh
<<file:__EnableUnstableFeatures('shape_destructure')>>

function f(shape('a' => int) $x): int {
  shape('a' => $i) = $x;
  return $i;
}
