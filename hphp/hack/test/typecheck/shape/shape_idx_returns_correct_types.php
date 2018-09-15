<?hh // strict

function shape_idx_nullable_field_with_default_return(
  shape('x' => ?int) $s,
): ?int {
  return Shapes::idx($s, 'x', 12);
}

function shape_idx_nullable_field_with_nullable_default_return(
  shape('x' => ?int) $s,
  ?int $default,
): ?int {
  return Shapes::idx($s, 'x', $default);
}

function shape_idx_nullable_optional_field_return(
  shape(?'x' => ?int) $s,
): ?int {
  return Shapes::idx($s, 'x');
}

function shape_idx_nullable_optional_field_with_default_return(
  shape(?'x' => ?int) $s,
  int $default,
): ?int {
  return Shapes::idx($s, 'x', $default);
}

function shape_idx_nullable_optional_field_with_nullable_default_return(
  shape(?'x' => ?int) $s,
  ?int $default,
): ?int {
  return Shapes::idx($s, 'x', $default);
}

function shape_idx_optional_field_with_nullable_default_return(
  shape(?'x' => int) $s,
  ?int $default,
): ?int {
  return Shapes::idx($s, 'x', $default);
}

function shape_idx_optional_nullable_field_with_default_return(
  shape(?'x' => ?int) $s,
): ?int {
  return Shapes::idx($s, 'x', 12);
}

function shape_idx_required_field_with_nullable_default_return(
  shape('x' => int) $s,
  ?int $default,
): ?int {
  return Shapes::idx($s, 'x', $default);
}
