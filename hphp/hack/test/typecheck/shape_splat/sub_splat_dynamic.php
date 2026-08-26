<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters', 'shape_splat_expression')>>

// Shape_splat <: dynamic
function to_dynamic<T as shape(...)>(
  shape(...T, 'name' => string) $s,
): dynamic {
  return $s;
}

function test_splat_dynamic(): void {
  $s = shape('id' => 42, 'name' => 'Alice');
  $d = to_dynamic($s);
}
