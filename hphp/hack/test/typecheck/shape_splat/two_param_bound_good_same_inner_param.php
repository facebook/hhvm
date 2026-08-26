<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>

// A row over `T2` can be returned unchanged when `T2` is bounded by `T1`.
function same_param_inner<T1 as shape(...), T2 as T1>(
  shape(...T2, 'q' => int) $s,
): shape(...T2, 'q' => int) {
  return $s;
}
