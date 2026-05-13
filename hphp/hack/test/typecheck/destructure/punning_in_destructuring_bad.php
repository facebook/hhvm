<?hh
<<file:__EnableUnstableFeatures('shape_and_tuple_destructuring', 'shape_field_punning')>>

// Punning with a field that doesn't exist
function test_unknown_field(shape('x' => int) $s): void {
  shape($x, $y) = $s; // 'y' does not exist
}
