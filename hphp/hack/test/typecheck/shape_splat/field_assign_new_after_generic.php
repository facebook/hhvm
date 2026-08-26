<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters', 'shape_splat_expression')>>

function add_field_after_generic<T as shape(...)>(
  shape('name' => string, ...T) $s,
): void {
  $s['brand_new'] = 5;
  hh_expect<int>($s['brand_new']);
  hh_expect<shape('name' => string, ...T, 'brand_new' => int)>($s);
}
