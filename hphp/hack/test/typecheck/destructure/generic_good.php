<?hh
<<file:__EnableUnstableFeatures('shape_and_tuple_destructuring')>>

// Generic with shape bound
function test_generic_shape_bound<T as shape('x' => int, 'y' => string, ...)>(
  T $s,
): void {
  shape('x' => $x, ...) = $s;
  hh_expect_equivalent<int>($x);
}

// Generic with tuple bound
function test_generic_tuple_bound<T as (int, string)>(T $t): void {
  tuple($a, $b) = $t;
  hh_expect_equivalent<int>($a);
  hh_expect_equivalent<string>($b);
}

// Intersection of shapes via multiple bounds
function test_intersection_shapes<
  T as shape('x' => int, ...) as shape('x' => int, 'z' => bool, ...),
>(T $s): void {
  shape('x' => $x, 'z' => $z, ...) = $s;
  hh_expect_equivalent<int>($x);
  hh_expect_equivalent<bool>($z);
}
