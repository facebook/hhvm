<?hh
<<file:__EnableUnstableFeatures('shape_and_tuple_destructuring')>>

// Open shape requires ellipsis in pattern
function test_open_shape_ok(shape('x' => int, ...) $s): void {
  shape('x' => $x, ...) = $s;
  hh_expect_equivalent<int>($x);
}

// Closed shape with ellipsis is ok
function test_closed_shape_with_ellipsis(shape('x' => int, 'y' => int) $s): void {
  shape('x' => $x, ...) = $s;
  hh_expect_equivalent<int>($x);
}

// Variadic tuple requires ellipsis in pattern
function test_variadic_tuple_ok((int, string...) $t): void {
  tuple($a, ...) = $t;
  hh_expect_equivalent<int>($a);
}
