<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters', 'shape_splat_expression')>>

function needs_name<T as shape(...)>(
  shape(...T, 'name' => string) $s,
): void {
  $_ = $s['name'];
}

function test(): void {
  // Error: shape missing 'name' field
  $s = shape('id' => 42);
  needs_name($s);
}
