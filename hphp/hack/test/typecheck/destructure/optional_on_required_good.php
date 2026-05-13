<?hh
<<file:__EnableUnstableFeatures('shape_and_tuple_destructuring')>>

function test_optional_on_required(shape('x' => int, 'y' => int) $s): void {
  shape('x' => $x, ?'y' => $y) = $s;
  hh_expect_equivalent<int>($x);
  hh_expect_equivalent<?int>($y);
}

function test_optional_on_required_open(
  shape('x' => int, 'y' => int, 'z' => bool) $s,
): void {
  shape('x' => $x, ?'y' => $y, ...) = $s;
  hh_expect_equivalent<int>($x);
  hh_expect_equivalent<?int>($y);
}
