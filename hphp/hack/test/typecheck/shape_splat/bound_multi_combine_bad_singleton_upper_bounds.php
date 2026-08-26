<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>

// The two upper bounds separately require `a` and `b`, but the return type
// requires both fields at once.
function lone<T as shape(...)>(
  shape(...T) $s,
): shape('a' => int, 'b' => bool, ...)
  where T as shape('a' => int, ...), T as shape('b' => bool, ...) {
  return $s;
}
