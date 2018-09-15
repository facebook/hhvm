<?hh // strict

// Error: should be ?int
function shape_idx_nullable_optional_field_return(shape(?'x' => ?int) $s): int {
  return Shapes::idx($s, 'x');
}
