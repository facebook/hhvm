<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters', 'shape_splat_expression')>>

function test_opt_not_req(shape(?'x' => int) $opt): shape('x' => int) {
  return $opt;
}
