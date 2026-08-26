<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>

function test_remove_key_missing_generic_splat<T as shape(...)>(
  shape('x' => int, ...T) $s,
): void {
  Shapes::removeKey(inout $s, 'z');
}
