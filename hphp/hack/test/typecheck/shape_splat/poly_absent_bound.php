<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters', 'shape_splat_expression')>>

// T is bounded to exclude 'name' via absent
function add_name<T as shape(?'name' => nothing, ...)>(
  T $s,
  string $name,
): shape(...T, 'name' => string) {
  return shape(...$s, 'name' => $name);
}

function test_absent_bound(): void {
  // Valid: input has no 'name' field
  $s = shape('id' => 42, 'age' => 30);
  $result = add_name($s, 'Alice');
  hh_expect<shape('age' => int, 'id' => int, 'name' => string)>($result);
}

function test_absent_bound_conflict(): void {
  // Error: input already has 'name' => int, violates absent bound
  $s = shape('id' => 42, 'name' => 123);
  add_name($s, 'Alice');
}
