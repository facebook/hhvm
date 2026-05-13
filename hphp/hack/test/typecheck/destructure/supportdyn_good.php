<?hh
<<file:__EnableUnstableFeatures('shape_and_tuple_destructuring')>>

// supportdyn<shape(...)>
function test_supportdyn_shape(supportdyn<shape('x' => int, 'y' => string)> $s): void {
  shape('x' => $x, 'y' => $y) = $s;
  hh_expect_equivalent<int>($x);
  hh_expect_equivalent<string>($y);
}

// supportdyn tuple
function test_supportdyn_tuple(supportdyn<(int, string)> $t): void {
  tuple($a, $b) = $t;
  hh_expect_equivalent<int>($a);
  hh_expect_equivalent<string>($b);
}

// supportdyn with optional field
function test_supportdyn_optional(
  supportdyn<shape('x' => int, ?'y' => string)> $s,
): void {
  shape('x' => $x, ?'y' => $y) = $s;
  hh_expect_equivalent<int>($x);
  hh_expect_equivalent<?string>($y);
}
