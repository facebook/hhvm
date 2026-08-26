<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters', 'shape_splat_expression')>>

// Tvar rightmost in sub: shape(...C, ...T) <: F
// T is dominant (rightmost), so T's fields override C's.
// With T as shape(...), 'id' could come from T with any type,
// so $s['id'] is mixed — this is correct behavior.
function extract_rest<T as shape(...)>(
  shape('id' => int, ...T) $s,
): void {
  // 'id' is in the concrete part (leftmost), but T is rightmost
  // and T might also have 'id' with a different type.
  // So $s['id'] is mixed, not int.
  hh_expect<shape('id' => int, ...T)>($s);
}

function test_tvar_rightmost_sub(): void {
  $s = shape('id' => 42, 'name' => 'Alice', 'age' => 30);
  extract_rest($s);
}
