<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters', 'shape_splat_expression')>>

// Expression-level splat should preserve type parameter identity
function add_field<T as shape(...)>(
  T $s,
): shape(...T, 'added' => bool) {
  return shape(...$s, 'added' => true);
}

function test_expr_preserve(): void {
  $input = shape('x' => 1, 'y' => 'hello');
  $result = add_field($input);
  hh_expect<shape('added' => bool, 'x' => int, 'y' => string)>($result);
}
