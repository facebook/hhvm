<?hh
<<file:__EnableUnstableFeatures('shape_and_tuple_destructuring')>>

// Test nullable propagation through nesting

// Tuple with shape child, all required
function test_tuple_with_shape_child(
  (shape('x' => int, 'y' => string),) $t,
): void {
  tuple(shape('x' => $x, 'y' => $y)) = $t;
  hh_expect_equivalent<int>($x);
  hh_expect_equivalent<string>($y);
}

// Deep nesting all required
function test_deep_nesting_required(
  shape('a' => shape('b' => shape('c' => int))) $s,
): void {
  shape('a' => shape('b' => shape('c' => $val))) = $s;
  hh_expect_equivalent<int>($val);
}

// Shape containing tuple
function test_shape_with_tuple_child(
  shape('pair' => (int, string)) $s,
): void {
  shape('pair' => tuple($a, $b)) = $s;
  hh_expect_equivalent<int>($a);
  hh_expect_equivalent<string>($b);
}

// Nested shape in tuple in shape
function test_deep_mixed_nesting(
  shape('items' => (shape('name' => string), int)) $s,
): void {
  shape('items' => tuple(shape('name' => $name), $count)) = $s;
  hh_expect_equivalent<string>($name);
  hh_expect_equivalent<int>($count);
}

// Deep nesting: shape -> shape -> tuple -> shape (3+ levels)
function test_deep_mixed_nesting_three_levels(
  shape('a' => shape('b' => (int, shape('c' => string)))) $deep,
): void {
  shape('a' => shape('b' => tuple($x, shape('c' => $y)))) = $deep;
  hh_expect_equivalent<int>($x);
  hh_expect_equivalent<string>($y);
}

// Mixed shape+tuple nesting
function test_mixed_shape_tuple(
  shape('pair' => (int, string), 'inner' => shape('z' => bool)) $s,
): void {
  shape('pair' => tuple($a, $b), 'inner' => shape('z' => $z)) = $s;
  hh_expect_equivalent<int>($a);
  hh_expect_equivalent<string>($b);
  hh_expect_equivalent<bool>($z);
}

// Optional tuple entry with nested shape
function test_optional_tuple_with_nested_shape(
  (shape('x' => int), string, float) $t,
): void {
  tuple(shape('x' => $x), optional $b, ...) = $t;
  hh_expect_equivalent<int>($x);
  hh_expect_equivalent<?string>($b);
}

// Deep nesting (3 levels) all required, with type assertions
function test_deep_nesting_required_with_assertions(
  shape('a' => shape('b' => shape('c' => int))) $s,
): void {
  shape('a' => shape('b' => shape('c' => $val))) = $s;
  hh_expect_equivalent<int>($val);
}

// Deep nesting all optional
function test_deep_nesting_optional(
  shape(?'a' => shape(?'b' => shape(?'c' => int, ...), ...), ...) $s,
): void {
  shape(?'a' => shape(?'b' => shape(?'c' => $val, ...), ...), ...) = $s;
  hh_expect_equivalent<?int>($val);
}
