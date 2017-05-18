<?hh // strict

function idxLocallyCreatedShape(): void {
  $shape = shape('x' => 0);

  Shapes::idx($shape, 'x');
  Shapes::idx($shape, 'y');
}
