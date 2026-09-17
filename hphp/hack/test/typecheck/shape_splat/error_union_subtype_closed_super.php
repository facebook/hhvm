<?hh
<<file:__EnableUnstableFeatures(
  'shape_splat_concrete',
  'shape_splat_type_parameters',
  'union_intersection_type_hints',
)>>

// BAD: expected to fail subtyping. Open-bounded members may carry extra unknown
// fields, so neither branch is a subtype of a CLOSED super — the union is
// rejected (both branches).

function sink_closed(shape('x' => int) $_): void {}

function closed_super<
  T1 as shape('x' => int, ...),
  T2 as shape('x' => int, ...),
>(shape(...(shape(...T1) | shape(...T2))) $s): void {
  sink_closed($s);
}
