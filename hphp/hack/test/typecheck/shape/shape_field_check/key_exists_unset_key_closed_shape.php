<?hh

function test(): bool {
  $s = shape('x' => 42, 'y' => 'foo');
  Shapes::removeKey(inout $s, 'x');
  return Shapes::keyExists($s, 'x');
}
