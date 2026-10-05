<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters', 'shape_splat_expression')>>

function test(shape('a' => int) $base): void {
  shape(...$base,/*range-start*//*range-end*/'b' => 2);
}
