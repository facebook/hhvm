<?hh
<<file:__EnableUnstableFeatures('shape_and_tuple_destructuring')>>

function f(shape('a' => int) $x): void {
  shape('a' => $i) = $x;
  tuple($x, $y) = tuple(1, 2);
  hh_expect_equivalent<int>($x);
  hh_expect_equivalent<int>($y);
  hh_expect_equivalent<int>($i);
}

// Empty shape destructuring
function test_empty_shape(shape() $s): void {
  shape() = $s;
}

// Empty shape on shape with only optional fields (closed pattern omits optional)
function test_empty_shape_optional(shape(?'x' => int) $s): void {
  shape() = $s;
}

// $_ is a placeholder variable (existing Hack semantics: reading it gives void)
function test_dollar_underscore_regular(
  shape('a' => int, 'b' => string) $s,
): void {
  shape('a' => $_, 'b' => $x) = $s;
  hh_expect_equivalent<string>($x);
}
