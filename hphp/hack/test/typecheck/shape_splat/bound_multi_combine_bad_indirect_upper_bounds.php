<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>

// The two upper bounds on `T2` separately require `a` and `b`, but the return
// type requires both fields at once.
function upper_two<T1 as T2, T2 as shape(...)>(
  shape(...T1) $s,
): shape('a' => int, 'b' => bool, ...)
  where T2 as shape('a' => int, ...), T2 as shape('b' => bool, ...) {
  return $s;
}
