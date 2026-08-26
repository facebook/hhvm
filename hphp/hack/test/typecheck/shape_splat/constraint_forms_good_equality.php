<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>

// Equality permits a row over either parameter to be returned as the same row
// over the other parameter.
function equal_both_ways<T1 as shape(...), T2 as shape(...)>(
  shape(...T2, 'q' => int) $s,
): shape(...T1, 'q' => int) where T1 = T2 {
  return $s;
}

function equal_reverse<T1 as shape(...), T2 as shape(...)>(
  shape(...T1, 'q' => int) $s,
): shape(...T2, 'q' => int) where T1 = T2 {
  return $s;
}
