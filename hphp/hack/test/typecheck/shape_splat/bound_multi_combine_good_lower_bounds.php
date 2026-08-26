<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>

// The two lower bounds allow values containing both `a` and `b`.
function lower_two<<<__Explicit>> T1 super T2 as shape(...), T2 as shape(...)>(
  shape('a' => int, 'b' => bool) $s,
): shape(...T1, ?'q' => bool)
  where T2 super shape('a' => int), T2 super shape('b' => bool) {
  return $s;
}
