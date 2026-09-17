<?hh
<<file:__EnableUnstableFeatures(
  'shape_splat_concrete',
  'shape_splat_type_parameters',
  'union_intersection_type_hints',
)>>

// Independent union splats distribute as a Cartesian product: three binary
// unions produce eight branches.
function cartesian_product<
  T1 as shape(...),
  T2 as shape(...),
  T3 as shape(...),
  T4 as shape(...),
  T5 as shape(...),
  T6 as shape(...),
>(
  shape(
    ...(shape(...T1) | shape(...T2)),
    ...(shape(...T3) | shape(...T4)),
    ...(shape(...T5) | shape(...T6)),
  ) $s,
): void {
  hh_show($s);
}
