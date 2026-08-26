<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters', 'shape_splat_expression')>>

// Simplest polymorphic shape function — identity
function identity<T as shape(...)>(
  shape(...T, 'name' => string) $s,
): shape(...T, 'name' => string) {
  return $s;
}

function test(): void {
  $user = shape('id' => 42, 'name' => 'Bob', 'age' => 30);
  $result = identity($user);
  hh_expect<shape('age' => int, 'id' => int, 'name' => string)>($result);
}
