<?hh
<<file:__EnableUnstableFeatures(
  'shape_splat_concrete',
  'shape_splat_type_parameters',
  'union_intersection_type_hints',
)>>

// Identical union members collapse (X | X ~> X) before normalization, so there
// is no distribution — a single residual splat results.
function collapse<T1 as shape(...)>(
  shape(...(shape(...T1) | shape(...T1)), 'c' => int) $s,
): void {
  hh_show($s);
}
