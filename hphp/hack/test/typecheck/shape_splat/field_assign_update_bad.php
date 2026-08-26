<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters', 'shape_splat_expression')>>

function assign_governed<T as shape('k' => arraykey)>(
  shape('k' => int, ...T) $s,
  string $v,
): shape('k' => int, ...T) {
  $s['k'] = $v;
  return $s;
}

function test_wrong_element(): void {
  $x = shape('k' => 'hello');
  $r = assign_governed($x, 'world');
  hh_expect<string>($r['k']);
}
