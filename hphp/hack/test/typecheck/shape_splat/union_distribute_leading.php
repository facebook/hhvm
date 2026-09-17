<?hh
<<file:__EnableUnstableFeatures(
  'shape_splat_concrete',
  'shape_splat_type_parameters',
  'union_intersection_type_hints',
)>>

// A leading concrete field appears in each distributed branch.
function leading<T1 as shape(...), T2 as shape(...)>(
  shape('c' => int, ...(shape(...T1) | shape(...T2))) $s,
): void {
  hh_show($s);
}
