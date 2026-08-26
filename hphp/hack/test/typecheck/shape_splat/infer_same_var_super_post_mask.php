<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>

// The explicit trailing `id` field in the return type remains required even
// when `T` may declare `id` as optional.
function same_var_super_post_masks<T as shape(?'id' => int, ...)>(
  shape('id' => int, ...T) $s,
): shape(...T, 'id' => int) {
  return $s;
}

function test(shape('id' => int, 'other' => string) $s): void {
  hh_expect<shape('id' => int, 'other' => string)>(same_var_super_post_masks($s));
}
