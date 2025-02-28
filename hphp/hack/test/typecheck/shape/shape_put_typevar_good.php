<?hh

function foo(): shape('y' => int, ...) {
  $vec = Vector {shape('y' => 'foo'), shape(), shape('x' => 11)};
  return Shapes::put($vec[0], 'y', 10);
}
