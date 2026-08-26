<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>

// Using `T1` as a field type in `T2`'s bound does not impose an ordering
// between the two shape rows.
function field_type_as<T1 as shape(...), T2 as shape('x' => T1)>(
  shape(...T2, 'q' => int) $s,
): shape(...T2, 'q' => int) {
  return $s;
}

function field_type_where<T1 as shape(...), T2 as shape(...)>(
  shape(...T2, 'q' => int) $s,
): shape(...T2, 'q' => int) where T2 as shape('x' => T1) {
  return $s;
}
