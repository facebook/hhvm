<?hh
<<file:__EnableUnstableFeatures(
  'shape_splat_concrete',
  'shape_splat_type_parameters',
  'union_intersection_type_hints',
)>>

// Concrete fields on both sides of the union appear in each branch.
function both_sides<T1 as shape(...), T2 as shape(...)>(
  shape('c' => int, ...(shape(...T1) | shape(...T2)), 'd' => bool) $s,
): void {
  hh_show($s);
}
