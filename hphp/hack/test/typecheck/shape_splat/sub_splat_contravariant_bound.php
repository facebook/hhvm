<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters', 'shape_splat_expression')>>

// Contravariant bound: T must have at least 'id' and 'email'
function needs_user_fields<T as shape('id' => int, 'email' => string, ...)>(
  shape(...T, 'role' => string) $s,
): void {
  hh_expect<string>($s['role']);
}

function test_ok(): void {
  $s = shape('id' => 1, 'email' => 'a@b.com', 'role' => 'admin', 'extra' => true);
  needs_user_fields($s);
}

function test_missing_field(): void {
  // Missing 'email' — should error against T's bound
  $s = shape('id' => 1, 'role' => 'admin');
  needs_user_fields($s);
}
