<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters', 'shape_splat_expression')>>

// Polymorphic field assignment preserves T's identity
function set_name<T as shape(...)>(
  shape(...T, 'name' => string) $s,
  string $new_name,
): shape(...T, 'name' => string) {
  $s['name'] = $new_name;
  return $s;
}

function test_field_assign(): void {
  $user = shape('id' => 42, 'name' => 'Alice', 'age' => 30);
  $result = set_name($user, 'Bob');
  hh_expect<shape('age' => int, 'id' => int, 'name' => string)>($result);
  hh_expect<int>($result['id']);
  hh_expect<string>($result['name']);
  hh_expect<int>($result['age']);
}
