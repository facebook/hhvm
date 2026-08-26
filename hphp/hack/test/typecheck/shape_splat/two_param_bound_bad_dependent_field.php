<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>

// `T2` inherits `b` as `bool` through `T1`, so an inline `b` of type `int`
// conflicts with its bound.
function dependent_field<T1 as shape('b' => bool), T2 as T1>(
  shape(...T2) $s,
): shape(...T2, 'b' => int) {
  return $s;
}
