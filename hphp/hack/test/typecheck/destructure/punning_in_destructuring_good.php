<?hh
<<file:__EnableUnstableFeatures('shape_and_tuple_destructuring', 'shape_field_punning')>>

function test_basic(shape('x' => int, 'y' => string) $s): void {
  shape($x, $y) = $s;
  hh_expect_equivalent<int>($x);
  hh_expect_equivalent<string>($y);
}

function test_optional(shape('x' => int, ?'y' => int) $s): void {
  shape($x, ?$y) = $s;
  hh_expect_equivalent<int>($x);
  hh_expect_equivalent<?int>($y);
}

function test_optional_ellipsis(
  shape('x' => int, ?'y' => int, ...) $s,
): void {
  shape(?$y, ...) = $s;
  hh_expect_equivalent<?int>($y);
}

function test_mixed_punned_and_explicit(
  shape('x' => int, 'y' => string) $s,
): void {
  shape($x, 'y' => $name) = $s;
  hh_expect_equivalent<int>($x);
  hh_expect_equivalent<string>($name);
}

function test_nested(
  shape('x' => shape('y' => int, 'z' => string)) $s,
): void {
  shape('x' => shape($y, $z)) = $s;
  hh_expect_equivalent<int>($y);
  hh_expect_equivalent<string>($z);
}

function test_nested_in_tuple(
  (shape('a' => int, 'b' => string), int) $t,
): void {
  tuple(shape($a, $b), $c) = $t;
  hh_expect_equivalent<int>($a);
  hh_expect_equivalent<string>($b);
  hh_expect_equivalent<int>($c);
}

function test_dollar_underscore(shape('_' => int) $s): void {
  shape($_) = $s;
}

function test_dollar_underscore_explicit(
  shape('_' => int) $s,
): void {
  shape('_' => $_) = $s;
}
