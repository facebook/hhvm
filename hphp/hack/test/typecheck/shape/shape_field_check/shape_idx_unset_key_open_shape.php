<?hh

function test(shape('x' => int, ...) $s): void {
  Shapes::removeKey(inout $s, 'x');
  Shapes::idx($s, 'x');
}
