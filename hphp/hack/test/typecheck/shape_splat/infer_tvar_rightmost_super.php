<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters', 'shape_splat_expression')>>

// Tvar rightmost in super: shape('x' => int, ...T)
// Concrete 'x' is leftmost, T is rightmost
function with_x<T as shape(...)>(
  shape('x' => int, ...T) $s,
): shape('x' => int, ...T) {
  return $s;
}

function test_tvar_rightmost_super(): void {
  $s = shape('x' => 1, 'y' => 'hello');
  $result = with_x($s);
  hh_expect<shape('x' => int, 'y' => string)>($result);
  hh_expect<int>($result['x']);
  hh_expect<string>($result['y']);
}
