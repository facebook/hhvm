<?hh

function test(shape('x' => shape('y' => string)) $shape): shape('x' => shape('y' => string, 'z' => string)) {
  return Shapes::put($shape, 'x', shape('z' => 'test'));
}
