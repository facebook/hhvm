<?hh

// Error: should be ?int
function shape_idx_optional_nullable_field_with_default_return(
  shape(?'x' => ?int) $s,
): int {
  return Shapes::idx($s, 'x', 12);
}
