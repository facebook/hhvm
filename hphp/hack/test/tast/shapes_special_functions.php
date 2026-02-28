<?hh

type Point = shape('x' => int, ?'y' => int);

function test(Point $p): void {
  Shapes::idx($p, 'x');
  Shapes::idx($p, 'x', 3);
  Shapes::keyExists($p, 'y');
  Shapes::removeKey(inout $p, 'y');
  Shapes::toArray($p);
}
