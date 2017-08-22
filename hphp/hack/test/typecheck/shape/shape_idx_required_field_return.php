<?hh // strict

function shape_idx_required_field_return1(): int {
  $s = shape('x' => 5);
  return Shapes::idx($s, 'x');
}

function shape_idx_required_field_return2(shape('x' => int) $s): int {
  return Shapes::idx($s, 'x');
}

function shape_idx_required_field_return3(
  shape(
    'x' => int,
    ...
  ) $s,
): int {
  return Shapes::idx($s, 'x');
}

function shape_idx_required_field_return4(
  shape('x' => int) $s,
  int $default,
): int {
  return Shapes::idx($s, 'x', $default);
}
