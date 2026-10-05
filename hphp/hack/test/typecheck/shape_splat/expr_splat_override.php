<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters', 'shape_splat_expression')>>

function test(): void {
  $a = shape('x' => 1, 'y' => true);
  $b = shape(...$a, 'x' => 'hello');
  hh_expect<shape('x' => string, 'y' => bool)>($b);
  // 'x' should be string (rightmost wins), 'y' should be bool
  hh_expect<string>($b['x']);
  hh_expect<bool>($b['y']);
}
