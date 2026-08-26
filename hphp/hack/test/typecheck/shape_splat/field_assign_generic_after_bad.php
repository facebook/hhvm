<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters', 'shape_splat_expression')>>

function clobber_after_generic<T as shape(...)>(
  shape('name' => string, ...T) $s,
): shape('name' => string, ...T) {
  $s['id'] = 'not an int';
  return $s;
}
