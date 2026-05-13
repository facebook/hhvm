<?hh
<<file:__EnableUnstableFeatures('shape_and_tuple_destructuring')>>

function test_shape_open(shape('x' => int, 'y' => int) $s): void {
  shape('x' => $x, ...) = $s;
  hh_expect_equivalent<int>($x);
}

function test_tuple_prefix((int, int, int) $t): void {
  tuple($a, ...) = $t;
  hh_expect_equivalent<int>($a);
}

function test_shape_bind_nothing(shape('x' => int) $s): void {
  shape(...) = $s;
}

function test_tuple_bind_nothing((int, int) $t): void {
  tuple(...) = $t;
}
