<?hh
<<file:__EnableUnstableFeatures(
  'shape_splat_concrete',
  'shape_splat_type_parameters',
  'union_intersection_type_hints',
)>>

function sink_open(shape('x' => int, ...) $_): void {}

// A trailing 'x' => int gives EVERY distributed branch 'x', so the union is a
// subtype of the super even though the members alone don't guarantee it. ACCEPT.
function accept_trailing<T1 as shape(...), T2 as shape(...)>(
  shape(...(shape(...T1) | shape(...T2)), 'x' => int) $s,
): void {
  sink_open($s);
}
