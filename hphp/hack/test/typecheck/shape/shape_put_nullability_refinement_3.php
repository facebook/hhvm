<?hh

function test(shape('x' => ?string) $shape): shape('x' => null) {
  return Shapes::put($shape, 'x', null);
}
