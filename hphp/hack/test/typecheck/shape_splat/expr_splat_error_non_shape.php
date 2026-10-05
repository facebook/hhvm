<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters', 'shape_splat_expression')>>

function test_non_shape_splat(int $x): void {
  $s = shape(...$x, 'y' => 1);
}
