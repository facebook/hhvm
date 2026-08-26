<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters', 'shape_splat_expression')>>

function overwrite_rightmost_concrete<T as shape(...)>(
  shape(...T, 'id' => arraykey) $s,
): void {
  $s['id'] = 5;
  hh_expect<int>($s['id']);
}
