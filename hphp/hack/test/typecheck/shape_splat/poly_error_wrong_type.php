<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters', 'shape_splat_expression')>>

// Error: passing wrong type for a known field
function needs_int_x<T as shape(...)>(
  shape(...T, 'x' => int) $s,
): void {}

function test_wrong_type(): void {
  $s = shape('x' => 'not_an_int', 'y' => true);
  needs_int_x($s); // Error: 'x' is string, expected int
}
