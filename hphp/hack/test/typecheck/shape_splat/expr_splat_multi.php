<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters', 'shape_splat_expression')>>

function test_multi_splat(): void {
  $a = shape('x' => 1, 'y' => 'hello');
  $b = shape('y' => true, 'z' => 3.14);
  $c = shape('z' => 'world');

  $result = shape(...$a, ...$b, ...$c);
  hh_expect<int>($result['x']);
  hh_expect<bool>($result['y']);
  hh_expect<string>($result['z']);
}
