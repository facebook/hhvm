<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters', 'shape_splat_expression')>>

// Restricted bound: T must have at least 'id' => int
function with_id<T as shape('id' => int, ...)>(
  shape(...T, 'name' => string) $s,
): shape(...T, 'name' => string) {
  return $s;
}

function test_restricted_bound(): void {
  // Valid: has 'id' => int plus extra fields
  $s = shape('id' => 42, 'name' => 'Alice', 'extra' => true);
  $result = with_id($s);
  hh_expect<shape('extra' => bool, 'id' => int, 'name' => string)>($result);
  hh_expect<int>($result['id']);
  hh_expect<string>($result['name']);
}

function test_restricted_bound_error(): void {
  // Error: missing 'id' field
  $s = shape('name' => 'Bob', 'x' => 1);
  with_id($s);
}
