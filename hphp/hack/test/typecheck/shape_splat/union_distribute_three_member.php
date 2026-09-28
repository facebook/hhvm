<?hh
<<file:__EnableUnstableFeatures(
  'shape_splat_concrete',
  'shape_splat_type_parameters',
  'union_intersection_type_hints',
)>>

// A three-member union distributes to three branches.
function three<T1 as shape(...), T2 as shape(...), T3 as shape(...)>(
  shape(...(T1 | T2 | T3), ...shape()) $s,
): void {
  hh_show($s);
}
