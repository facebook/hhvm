<?hh // strict

// Error: should be ?int
function shape_idx_nullable_field_with_nullable_default_return(
  shape('x' => ?int) $s,
  ?int $default,
): int {
  return Shapes::idx($s, 'x', $default);
}
