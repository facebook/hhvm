<?hh
<<file:__EnableUnstableFeatures(
  'shape_splat_concrete',
  'shape_splat_type_parameters',
  'union_intersection_type_hints',
)>>

// A union of a splat-shape and a simple shape distributes; the simple branch
// fully merges while the splat branch stays residual.
function mixed<T1 as shape(...)>(
  shape(...(shape(...T1) | shape('b' => int)), 'c' => int) $s,
): void {
  hh_show($s);
}
