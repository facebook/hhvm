<?hh

function test(shape('y' => num) $shape): shape('x' => string, 'y' => num) {
  return Shapes::put(inout $shape, 'x', 'test');
}
