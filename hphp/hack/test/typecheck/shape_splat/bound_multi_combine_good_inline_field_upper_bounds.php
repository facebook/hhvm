<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>

// The two upper bounds guarantee `a` and `b`, and the row explicitly requires
// `z`.
function two_elem<T as shape(...)>(
  shape(...T, 'z' => int) $s,
): shape('a' => int, 'b' => bool, 'z' => int, ...)
  where T as shape('a' => int, ...), T as shape('b' => bool, ...) {
  return $s;
}
