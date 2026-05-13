<?hh
<<file:__EnableUnstableFeatures('shape_and_tuple_destructuring')>>

function test_nested_shape_in_tuple(
  (shape('a' => int, 'b' => int), int) $t,
): void {
  tuple(shape('a' => $a, ...), ...) = $t;
  hh_expect_equivalent<int>($a);
}

function test_nested_tuple_in_shape(
  shape('t' => (int, int, int)) $s,
): void {
  shape('t' => tuple($x, ...)) = $s;
  hh_expect_equivalent<int>($x);
}

function test_nested_shape_in_shape(
  shape('inner' => shape('a' => int, 'b' => int), 'c' => int) $s,
): void {
  shape('inner' => shape('a' => $a, ...), ...) = $s;
  hh_expect_equivalent<int>($a);
}
