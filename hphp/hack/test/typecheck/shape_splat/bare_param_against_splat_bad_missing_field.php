<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>

// The bare parameter need not supply the row's inline field.
function missing<T as shape(...)>(T $s): shape(...T, 'a' => int) {
  return $s;
}
