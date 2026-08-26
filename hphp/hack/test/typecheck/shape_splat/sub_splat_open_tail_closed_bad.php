<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>

function open_tail_to_closed<T as shape(...)>(
  shape(...T) $s,
): shape() {
  return $s;
}
