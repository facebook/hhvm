<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters', 'shape_splat_expression')>>

// Splatting an open shape should preserve openness
function test_open_splat(shape('x' => int, ...) $open): void {
  $result = shape(...$open, 'y' => 'hello');
  hh_show($result);
}
