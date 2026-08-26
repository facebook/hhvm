<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters', 'shape_splat_expression')>>


function clobber<T as shape(...)>(
  shape(...T, 'name' => string) $s,
): shape(...T, 'name' => string) {
  $s['id'] = 'not an int';
  return $s;
}
