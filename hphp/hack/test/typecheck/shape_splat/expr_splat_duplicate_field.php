<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters', 'shape_splat_expression')>>

// Duplicate field names with splats — duplicates among SF_field entries
// should still be caught even when splats are present
function test_duplicate(): void {
  $a = shape('x' => 1);
  $s = shape(...$a, 'y' => 1, 'y' => 2);
}
