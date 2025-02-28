<?hh

function test(shape('y' => num) $shape): shape('y' => num) {
  $x = Shapes::put($shape, 'x', 'test');
  test2($x);
  return $shape;
}

function test2(shape('y' => num, 'x' => string) $shape): void {
}
