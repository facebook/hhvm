<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters', 'shape_splat_expression')>>

function update_name<T as shape(...)>(
  shape(...T, 'name' => string) $s,
): shape(...T, 'name' => string) {
  $s['name'] = $s['name'] . '!';
  return $s;
}

function test(): void {
  $user = shape('id' => 42, 'name' => 'Bob', 'age' => 30);
  $result = update_name($user);
  hh_expect<shape('age' => int, 'id' => int, 'name' => string)>($result);
}
