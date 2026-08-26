<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters', 'shape_splat_expression')>>

// Tvar leftmost in super: shape(...T, 'x' => int)
// T is leftmost (submissive), concrete 'x' is rightmost (dominant)
// No fresh vars needed for overlapping fields — concrete wins
function set_x<T as shape(...)>(
  shape(...T, 'x' => int) $s,
): shape(...T, 'x' => int) {
  return $s;
}

function test_leftmost_super(): void {
  // 'x' is string in input but int in the concrete part — should error
  $s = shape('x' => 'wrong', 'y' => true);
  set_x($s);
}

function test_leftmost_super_ok(): void {
  // 'x' is int — should be fine
  $s = shape('x' => 42, 'y' => true);
  $result = set_x($s);
  hh_expect<shape('x' => int, 'y' => bool)>($result);
  hh_expect<int>($result['x']);
  hh_expect<bool>($result['y']);
}
