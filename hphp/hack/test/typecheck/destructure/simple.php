<?hh
<<file:__EnableUnstableFeatures('shape_destructure')>>

function f(shape('a' => int) $x): void {
  shape('a' => $i) = $x;
  tuple($x, $y) = tuple(1, 2);
  hh_expect<int>($x);
  hh_expect<int>($y);
  hh_expect<int>($i);
}
