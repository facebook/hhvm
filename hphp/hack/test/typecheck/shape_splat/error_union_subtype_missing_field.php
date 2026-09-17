<?hh
<<file:__EnableUnstableFeatures(
  'shape_splat_concrete',
  'shape_splat_type_parameters',
  'union_intersection_type_hints',
)>>

// BAD: expected to fail subtyping. `shape(...(A|B)) <: C` requires BOTH branches
// to be subtypes. T2's residual need not carry the required 'x', so one branch
// fails and the union is rejected.

function sink_open(shape('x' => int, ...) $_): void {}

function missing_field<T1 as shape('x' => int), T2 as shape(...)>(
  shape(...(shape(...T1) | shape(...T2))) $s,
): void {
  sink_open($s);
}
