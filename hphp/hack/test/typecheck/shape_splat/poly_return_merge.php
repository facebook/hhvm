<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters', 'shape_splat_expression')>>

// Return type with merged fields — T preserves rest, concrete fields overridden
function with_timestamp<T as shape(...)>(
  T $s,
): shape(...T, 'updated_at' => int) {
  // T is the input, we add a field
  return shape(...$s, 'updated_at' => time());
}

function test_return_merge(): void {
  $input = shape('name' => 'Alice', 'age' => 30);
  $result = with_timestamp($input);
  hh_expect<shape('age' => int, 'name' => string, 'updated_at' => int)>($result);
  hh_expect<string>($result['name']);
  hh_expect<int>($result['updated_at']);
}
