<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>

// `T2 as shape(...T1)` makes `T2` a subtype of `T1`; it does not make a `T1`
// value satisfy `T2`, and `T1` also does not guarantee the required field `z`.
function via_bound<T1 as shape(...), <<__Explicit>> T2 as shape(...T1)>(
  T1 $s,
): shape(...T2, 'z' => int) {
  return $s;
}
