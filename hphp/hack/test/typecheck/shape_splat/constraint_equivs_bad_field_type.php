<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>

// `T2` requires `b` to be `bool`, so neither constraint spelling permits an
// inline `b` field of type `int`.
function field_as<T1 as shape('b' => bool), T2 as T1>(
  shape(...T2) $s,
): shape(...T2, 'b' => int) {
  return $s;
}

function field_super<T2 as shape('b' => bool), T1 super T2 as shape('b' => bool)>(
  shape(...T2) $s,
): shape(...T2, 'b' => int) {
  return $s;
}
