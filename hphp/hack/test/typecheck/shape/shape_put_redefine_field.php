<?hh

function test(shape('x' => int) $shape): shape('x' => string) {
  return Shapes::put($shape, 'x', 'test');
}
