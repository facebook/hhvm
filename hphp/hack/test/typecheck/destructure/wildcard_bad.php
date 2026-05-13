<?hh
<<file:__EnableUnstableFeatures('shape_and_tuple_destructuring')>>

// Wildcard not allowed in type position
function test_wildcard_in_type(shape('x' => int, 'y' => _) $s): void {}

function test_wildcard_punning(shape('x' => int) $s): void {
  shape(_) = $s; // error
}

function test_wildcard_in_shape_expr(): void {
  $s = shape('x' => _); // error
}

function test_wildcard_assign(): void {
  $x = _; // error
}
