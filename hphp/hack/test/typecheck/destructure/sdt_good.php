<?hh
<<file:__EnableUnstableFeatures('shape_and_tuple_destructuring')>>

// SDT functions re-check the body with params widened to dynamic.
// Structural destructuring checks must not report spurious errors during that
// dynamic re-check.

<<__SupportDynamicType>>
function test_sdt_closed_shape(shape('x' => int, 'y' => string) $s): void {
  shape('x' => $x, 'y' => $y) = $s;
  hh_expect_equivalent<int>($x);
  hh_expect_equivalent<string>($y);
}

<<__SupportDynamicType>>
function test_sdt_open_shape(shape('x' => int, 'y' => string) $s): void {
  shape('x' => $x, ...) = $s;
  hh_expect_equivalent<int>($x);
}

<<__SupportDynamicType>>
function test_sdt_tuple((int, string) $t): void {
  tuple($a, $b) = $t;
  hh_expect_equivalent<int>($a);
  hh_expect_equivalent<string>($b);
}
