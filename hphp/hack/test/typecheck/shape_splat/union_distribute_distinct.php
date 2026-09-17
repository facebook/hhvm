<?hh
<<file:__EnableUnstableFeatures(
  'shape_splat_concrete',
  'shape_splat_type_parameters',
  'union_intersection_type_hints',
)>>

// A union of splat shapes with distinct type parameters cannot be joined by
// `hh`, so it distributes: shape(...(A|B)) ~> shape(...A) | shape(...B).
function distinct<T1 as shape(...), T2 as shape(...)>(
  shape(...(shape(...T1) | shape(...T2))) $s,
): void {
  hh_show($s);
}
