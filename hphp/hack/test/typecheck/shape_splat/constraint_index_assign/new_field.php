<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters', 'shape_splat_expression')>>

// Writing a new field is accepted, and reading it has the assigned type.
function add_field<T as shape(...)>(
  shape(...T, 'name' => string) $s,
): void {
  $s['brand_new'] = 5;
  hh_expect<int>($s['brand_new']);
}
