<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>

// `shape(...T)` normalizes to `T`, so the two spellings of the same closed row
// must decide subtyping identically in either direction.
function id_fwd<T as shape()>(T $s): shape(...T) {
  return $s;
}

function id_rev<T as shape()>(shape(...T) $s): T {
  return $s;
}
