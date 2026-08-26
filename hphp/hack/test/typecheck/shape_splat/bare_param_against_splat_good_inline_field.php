<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>

// The inline field appears on both sides.
function inline_both<T as shape(...)>(
  shape(...T, 'a' => int) $s,
): shape(...T, 'a' => int) {
  return $s;
}
