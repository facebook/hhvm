<?hh
<<file:__EnableUnstableFeatures('shape_and_tuple_destructuring')>>

// Foreach with basic shape
function test_foreach_shape(vec<shape('x' => int, 'y' => string)> $points): void {
  foreach ($points as shape('x' => $x, 'y' => $y)) {
    hh_expect_equivalent<int>($x);
    hh_expect_equivalent<string>($y);
  }
}

// Foreach with optional field
function test_foreach_optional(
  vec<shape('x' => int, ?'y' => string)> $items,
): void {
  foreach ($items as shape('x' => $x, ?'y' => $y)) {
    hh_expect_equivalent<int>($x);
    hh_expect_equivalent<?string>($y);
  }
}

// Foreach with tuple
function test_foreach_tuple(vec<(int, string)> $pairs): void {
  foreach ($pairs as tuple($a, $b)) {
    hh_expect_equivalent<int>($a);
    hh_expect_equivalent<string>($b);
  }
}

// Foreach key-value with shape destructuring
function test_foreach_kv(
  dict<string, shape('x' => int, 'y' => int)> $map,
): void {
  foreach ($map as $key => shape('x' => $x, 'y' => $y)) {
    hh_expect_equivalent<string>($key);
    hh_expect_equivalent<int>($x);
    hh_expect_equivalent<int>($y);
  }
}

// Foreach with shape destructuring on the key side
function test_foreach_shape_key(
  KeyedIterator<shape('id' => int, 'label' => string), (int, string)> $items,
): void {
  foreach ($items as shape('id' => $id, 'label' => $label) => tuple($num, $name)) {
    hh_expect_equivalent<int>($id);
    hh_expect_equivalent<string>($label);
    hh_expect_equivalent<int>($num);
    hh_expect_equivalent<string>($name);
  }
}

// Foreach with tuple destructuring on the key side
function test_foreach_tuple_key(
  KeyedIterator<(int, string), shape('ok' => bool)> $items,
): void {
  foreach ($items as tuple($id, $label) => shape('ok' => $ok)) {
    hh_expect_equivalent<int>($id);
    hh_expect_equivalent<string>($label);
    hh_expect_equivalent<bool>($ok);
  }
}

// Foreach with nested shape+tuple
function test_foreach_nested(
  vec<(shape('a' => int), int)> $items,
): void {
  foreach ($items as tuple(shape('a' => $a), $b)) {
    hh_expect_equivalent<int>($a);
    hh_expect_equivalent<int>($b);
  }
}

// Nested optional shape fields in foreach
function test_foreach_nested_optional(
  vec<shape('a' => shape('x' => int, ?'y' => string))> $items,
): void {
  foreach ($items as shape('a' => shape('x' => $x, ?'y' => $y, ...))) {
    hh_expect_equivalent<int>($x);
    hh_expect_equivalent<?string>($y);
  }
}
