<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>

// `T2 <: T1`, so a row over `T2` can be returned as the same row over `T1`.
function bounded_by_param<T1 as shape(...), T2 as T1>(
  shape(...T2, 'q' => int) $s,
): shape(...T1, 'q' => int) {
  return $s;
}
