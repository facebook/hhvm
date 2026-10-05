<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters', 'shape_splat_expression')>>

// Chained polymorphic calls — T flows through
function add_x<T as shape(...)>(
  T $s,
): shape(...T, 'x' => int) {
  return shape(...$s, 'x' => 42);
}

function add_y<T as shape(...)>(
  T $s,
): shape(...T, 'y' => string) {
  return shape(...$s, 'y' => 'hello');
}

function test_nested(): void {
  $base = shape('id' => 1);
  $with_x = add_x($base);
  $with_xy = add_y($with_x);
  hh_expect<shape('id' => int, 'x' => int, 'y' => string)>($with_xy);
}
