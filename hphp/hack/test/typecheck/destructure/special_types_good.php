<?hh
<<file:__EnableUnstableFeatures('shape_and_tuple_destructuring')>>

// nothing RHS -- all variables receive nothing
function test_nothing_shape(nothing $s): void {
  shape('x' => $x, 'y' => $y) = $s;
  hh_expect_equivalent<nothing>($x);
  hh_expect_equivalent<nothing>($y);
}

function test_nothing_tuple(nothing $t): void {
  tuple($a, $b) = $t;
  hh_expect_equivalent<nothing>($a);
  hh_expect_equivalent<nothing>($b);
}

// dynamic RHS -- all variables receive dynamic
function test_dynamic_shape(dynamic $s): void {
  shape('x' => $x, 'y' => $y, ...) = $s;
  hh_expect_equivalent<dynamic>($x);
  hh_expect_equivalent<dynamic>($y);
}

function test_dynamic_tuple(dynamic $t): void {
  tuple($a, $b) = $t;
  hh_expect_equivalent<dynamic>($a);
  hh_expect_equivalent<dynamic>($b);
}
