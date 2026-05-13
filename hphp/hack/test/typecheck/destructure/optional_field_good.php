<?hh
<<file:__EnableUnstableFeatures('shape_and_tuple_destructuring')>>

function test_optional_explicit(
  shape('x' => int, ?'y' => int) $s,
): void {
  shape('x' => $x, ?'y' => $y) = $s;
  hh_expect_equivalent<int>($x);
  hh_expect_equivalent<?int>($y);
}

function test_optional_with_ellipsis(
  shape('x' => int, ?'y' => int, ...) $s,
): void {
  shape('x' => $x, ?'y' => $y, ...) = $s;
  hh_expect_equivalent<int>($x);
  hh_expect_equivalent<?int>($y);
}

// Closed pattern omitting optional fields is OK
function test_closed_omit_optional(
  shape('a' => int, 'b' => int, ?'c' => int) $s,
): void {
  shape('a' => $a, 'b' => $b) = $s;
  hh_expect_equivalent<int>($a);
  hh_expect_equivalent<int>($b);
}
