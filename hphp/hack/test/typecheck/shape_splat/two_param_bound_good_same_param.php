<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>

// A row can be returned with the same spread parameter.
function same_param<T1 as shape(...)>(
  shape(...T1, 'q' => int) $s,
): shape(...T1, 'q' => int) {
  return $s;
}
