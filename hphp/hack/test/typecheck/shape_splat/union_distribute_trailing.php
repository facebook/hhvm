<?hh
<<file:__EnableUnstableFeatures(
  'shape_splat_concrete',
  'shape_splat_type_parameters',
  'union_intersection_type_hints',
)>>

// A trailing concrete field is merged into each distributed branch.
function trailing<T1 as shape(...), T2 as shape(...)>(
  shape(...(T1 | T2), 'c' => int) $s,
): void {
  hh_show($s);
}
