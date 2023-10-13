<?hh

// Error: should be ?int
function shape_idx_nullable_optional_field_with_default_return(
  shape(?'x' => ?int) $s,
  int $default,
): int {
  return Shapes::idx($s, 'x', $default);
}
