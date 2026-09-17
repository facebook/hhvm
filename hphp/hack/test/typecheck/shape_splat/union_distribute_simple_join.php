<?hh
<<file:__EnableUnstableFeatures(
  'shape_splat_concrete',
  'shape_splat_type_parameters',
  'union_intersection_type_hints',
)>>

// A union of SIMPLE (param-free) shapes is joined by `hh` before normalization,
// so it never reaches distribution — the result is a single joined shape.
function simple(
  shape(...(shape('a' => int) | shape('b' => string)), 'c' => bool) $s,
): void {
  hh_show($s);
}
