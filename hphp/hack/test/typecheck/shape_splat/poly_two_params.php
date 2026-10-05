<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters', 'shape_splat_expression')>>

// Two separate type params, each with sole-splat occurrence
function merge_shapes<T1 as shape(...), T2 as shape(...)>(
  shape(...T1, 'x' => int) $a,
  shape(...T2, 'y' => int) $b,
): shape(...T1, ...T2, 'x' => mixed, 'y' => int) {
  return shape(...$a, ...$b);
}

function test_two_params(): void {
  $a = shape('x' => 1, 'a_field' => 'hello');
  $b = shape('y' => 2, 'b_field' => true);
  $result = merge_shapes($a, $b);
  hh_expect<shape('a_field' => string, 'b_field' => bool, 'x' => mixed, 'y' => int)>($result);
}
