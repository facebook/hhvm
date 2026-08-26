<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>

function optional_bound_to_required<T as shape(?'id' => int, ...)>(
  shape(...T) $s,
): shape('id' => int, ...) {
  return $s;
}
