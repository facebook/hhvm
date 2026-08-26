<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>

// A direct lower bound and the equivalent lower bound through another type
// parameter must allow the same shape-splat subtype check.

// The lower bound is a shape, readable directly.
function direct<<<__Explicit>> T1 super shape('a' => int) as shape(...)>(
  shape('a' => int) $s,
): shape(...T1, ?'q' => bool) {
  return $s;
}

// The same constraint on `T1`, reached through one parameter hop.
function via_chain<
  T2 super shape('a' => int) as shape(...),
  <<__Explicit>> T1 super T2 as shape(...),
>(shape('a' => int) $s): shape(...T1, ?'q' => bool) {
  return $s;
}
