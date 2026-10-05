<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters', 'shape_splat_expression')>>

function test_nested_splat(): void {
  $a = shape('x' => 1);
  $b = shape('y' => 'hello');
  $inner = shape(...$a, ...$b);
  $result = shape(...$inner, 'z' => true);
  hh_show($result);
  hh_expect<int>($result['x']);
  hh_expect<string>($result['y']);
  hh_expect<bool>($result['z']);
}
