<?hh

function shape_idx_required_field_with_default_return(
  shape('x' => int) $s,
): int {
  return Shapes::idx($s, 'x', 12);
}
