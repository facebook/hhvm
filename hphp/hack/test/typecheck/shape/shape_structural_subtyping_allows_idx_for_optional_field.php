<?hh // strict

type ShapeWithOptionalField = shape(?'a' => int);

function foo(ShapeWithOptionalField $shape): void {
  Shapes::idx($shape, 'a');
}
