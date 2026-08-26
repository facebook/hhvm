<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>

// Both constraint spellings establish `T2 <: T1`, so a row over `T2` can be
// returned as the same row over `T1`.
function accept_as<T1 as shape(...), T2 as T1>(
  shape(...T2, 'q' => int) $s,
): shape(...T1, 'q' => int) {
  return $s;
}

function accept_super<
  T2 as shape(...),
  <<__Explicit>> T1 super T2 as shape(...),
>(
  shape(...T2, 'q' => int) $s,
): shape(...T1, 'q' => int) {
  return $s;
}
