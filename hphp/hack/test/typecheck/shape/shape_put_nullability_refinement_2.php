<?hh

function test(shape('x' => ?string) $shape): shape('x' => string) {
  return Shapes::put($shape, 'x', 'test');
}
