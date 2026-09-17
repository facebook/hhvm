<?hh
<<file:__EnableUnstableFeatures(
  'shape_splat_concrete',
  'shape_splat_type_parameters',
  'union_intersection_type_hints',
)>>

// BAD: expected to fail subtyping. Both branches carry 'x' => int, but the super
// requires 'x' => string, so every branch has the wrong field type and the union
// is rejected.

function sink_x_string(shape('x' => string, ...) $_): void {}

function wrong_type<T1 as shape('x' => int), T2 as shape('x' => int)>(
  shape(...(shape(...T1) | shape(...T2))) $s,
): void {
  sink_x_string($s);
}
