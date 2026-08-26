<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>

// Equal parameters that require `b` as `bool` cannot be combined with an
// inline `b` field of type `int`.
function equality_cycle<T1 as shape('b' => bool), T2 as shape('b' => bool)>(
  shape(...T2) $s,
): shape(...T2, 'b' => int) where T1 = T2 {
  return $s;
}
