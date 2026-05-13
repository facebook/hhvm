<?hh
<<file:__EnableUnstableFeatures('shape_and_tuple_destructuring')>>

// Nested optional fields should parse and type-check correctly.

function test_nested_optional_in_shape(
  shape('outer' => shape('x' => int, ?'y' => string)) $s,
): void {
  shape('outer' => shape('x' => $x, ?'y' => $y, ...)) = $s;
  hh_expect_equivalent<int>($x);
  hh_expect_equivalent<?string>($y);
}

function test_nested_optional_in_tuple(
  (shape('a' => int, ?'b' => float),) $t,
): void {
  tuple(shape('a' => $a, ?'b' => $b, ...)) = $t;
  hh_expect_equivalent<int>($a);
  hh_expect_equivalent<?float>($b);
}
