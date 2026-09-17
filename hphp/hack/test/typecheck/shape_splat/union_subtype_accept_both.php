<?hh
<<file:__EnableUnstableFeatures(
  'shape_splat_concrete',
  'shape_splat_type_parameters',
  'union_intersection_type_hints',
)>>

function sink_open(shape('x' => int, ...) $_): void {}

// Both branches guarantee 'x' => int, so the distributed union is a subtype of
// the open super ((a|b) <: c iff a<:c and b<:c). ACCEPT (no error).
function accept_both<T1 as shape('x' => int), T2 as shape('x' => int)>(
  shape(...(shape(...T1) | shape(...T2))) $s,
): void {
  sink_open($s);
}
