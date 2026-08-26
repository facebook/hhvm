<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>

function test_to_array_generic_splat<T as shape('y' => string, ...)>(
  shape('x' => int, ...T) $s,
): void {
  hh_expect_equivalent<dict<arraykey, mixed>>(Shapes::toArray($s));
  hh_expect_equivalent<dict<arraykey, mixed>>(Shapes::toDict($s));
}
