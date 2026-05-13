<?hh
<<file:__EnableUnstableFeatures('shape_and_tuple_destructuring', 'shape_field_punning')>>

function test_shape_basic(shape('x' => int, 'y' => int) $s): void {
  shape('x' => $x, 'y' => $y) = $s;
}

function test_shape_ellipsis(shape('x' => int, 'y' => int) $s): void {
  shape('x' => $x, ...) = $s;
}

function test_shape_optional(shape('x' => int, ?'y' => int) $s): void {
  shape('x' => $x, ?'y' => $y) = $s;
}

function test_shape_wildcard(shape('x' => int, 'y' => int) $s): void {
  shape('x' => $x, 'y' => _) = $s;
}

function test_shape_punning(shape('x' => int, 'y' => int) $s): void {
  shape($x, $y) = $s;
}

function test_tuple_basic((int, string) $t): void {
  tuple($a, $b) = $t;
}

function test_tuple_ellipsis((int, string, bool) $t): void {
  tuple($a, ...) = $t;
}

function test_nested(
  shape('inner' => (int, string)) $s,
): void {
  shape('inner' => tuple($a, $b)) = $s;
}

<<__NoAutoDynamic>>
function test_foreach_shape_key(
  KeyedIterator<shape('id' => int), (int, string)> $items,
): void {
  foreach ($items as shape('id' => $id) => tuple($a, $b)) {}
}

<<__NoAutoDynamic>>
function test_foreach_tuple_key(
  KeyedIterator<(int, string), shape('ok' => bool)> $items,
): void {
  foreach ($items as tuple($id, $label) => shape('ok' => $ok)) {}
}
