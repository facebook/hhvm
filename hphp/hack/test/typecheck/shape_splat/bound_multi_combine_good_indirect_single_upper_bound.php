<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>

// The upper bound on `T2` guarantees the field required by the return type.
function upper_one<T1 as T2, T2 as shape(...)>(
  shape(...T1) $s,
): shape('a' => int, ...)
  where T2 as shape('a' => int, ...) {
  return $s;
}
