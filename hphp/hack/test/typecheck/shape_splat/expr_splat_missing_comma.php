<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters', 'shape_splat_expression')>>

function missing_comma(shape('a' => int) $base): void {
  $result = shape(...$base 'b' => 2);
}
