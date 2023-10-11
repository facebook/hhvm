<?hh

function test(): bool {
  $s = shape('x' => 42);
  return Shapes::keyExists($s, 'x');
}
