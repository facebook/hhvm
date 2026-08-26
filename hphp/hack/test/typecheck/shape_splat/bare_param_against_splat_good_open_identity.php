<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>

// The two spellings remain equivalent with an open bound, where `T` may carry
// unknown fields.
function id_open_fwd<T as shape(...)>(T $s): shape(...T) {
  return $s;
}

function id_open_rev<T as shape(...)>(shape(...T) $s): T {
  return $s;
}
