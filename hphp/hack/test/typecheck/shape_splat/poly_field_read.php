<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters', 'shape_splat_expression')>>

// Field read on polymorphic shape — known field has concrete type
function get_name<T as shape(...)>(
  shape(...T, 'name' => string) $s,
): string {
  return $s['name'];
}

function test_field_read(): void {
  $user = shape('id' => 42, 'name' => 'Alice');
  $name = get_name($user);
  hh_expect<string>($name);
}
