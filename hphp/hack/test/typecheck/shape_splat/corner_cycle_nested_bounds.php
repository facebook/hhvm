<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>

// REJECT. The cyclic bounds permit T1 and T2 to both be
// shape('x' => int), so T1's optional x is not always a string.
function cyclic_nested_bounds<
  T1 as shape(...T2),
  T2 as shape(...T1),
>(
  shape(...T1, 'q' => int) $value,
): shape(?'x' => string, 'q' => int) {
  return $value;
}

function witness_cycle(
  shape('x' => int, 'q' => int) $value,
): shape(?'x' => string, 'q' => int) {
  return cyclic_nested_bounds<
    shape('x' => int),
    shape('x' => int),
  >($value);
}
