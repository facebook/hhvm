<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>

// Neither constraint spelling establishes `T1 <: T2`, so a row over `T1`
// cannot be returned as the same row over `T2`.
function reject_as<T1 as shape(...), <<__Explicit>> T2 as T1>(
  shape(...T1, 'q' => int) $s,
): shape(...T2, 'q' => int) {
  return $s;
}

function reject_super<T2 as shape(...), T1 super T2 as shape(...)>(
  shape(...T1, 'q' => int) $s,
): shape(...T2, 'q' => int) {
  return $s;
}
