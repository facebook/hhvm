<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>

// Deeply nested splats with multiple abstract generics (the motivating
// example). Normalization must flatten the nesting: each maximal run of
// concrete fields collapses into one simple shape, interleaved with the
// abstract generics in source order. No Shape_splat nests inside another.
function nested_deep<T1 as shape(...), T2 as shape(...)>(
  shape(
    'x' => int,
    ...shape('y' => bool, ...T1),
    ...shape('z' => string, ...shape('u' => float, ...T2)),
  ) $s,
): void {
  hh_show($s);
}
