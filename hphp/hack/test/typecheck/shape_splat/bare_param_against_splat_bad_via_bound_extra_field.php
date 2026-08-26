<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>

// Instantiating both parameters at `shape('a' => int)` leaves
// `shape('a' => int, 'z' => int) <: shape('a' => int)`, which a closed shape
// does not admit.
function via_bound_rev<T1 as shape(...), <<__Explicit>> T2 as shape(...T1)>(
  shape(...T2, 'z' => int) $s,
): T1 {
  return $s;
}
