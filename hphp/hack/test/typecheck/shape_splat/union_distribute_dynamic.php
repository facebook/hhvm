<?hh
<<file:__EnableUnstableFeatures(
  'shape_splat_concrete',
  'shape_splat_type_parameters',
  'union_intersection_type_hints',
)>>

// Union distribution composes with a `dynamic` operand: each branch carries the
// open `dynamic` row.
function with_dynamic<T1 as shape(...), T2 as shape(...)>(
  shape(...(shape(...T1) | shape(...T2)), ...dynamic) $s,
): void {
  hh_show($s);
}
