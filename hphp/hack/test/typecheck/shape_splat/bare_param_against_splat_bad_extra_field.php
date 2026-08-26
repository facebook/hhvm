<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>

// The row adds a field the bare parameter need not admit.
function extra<T as shape(...)>(shape(...T, 'a' => int) $s): T {
  return $s;
}
