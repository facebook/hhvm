<?hh

function shape_idx_optional_field_default_return(shape(?'x' => int) $s): int {
  return Shapes::idx($s, 'x', 12);
}
