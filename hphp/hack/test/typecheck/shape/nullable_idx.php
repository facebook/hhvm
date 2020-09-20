<?hh

function takes_nullable_shape(?shape('x' => int) $val): ?int {
  return Shapes::idx($val, 'x', 0);
}
