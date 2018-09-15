<?hh // strict

type ShapeWithNullableField = shape('a' => ?int);

function foo(ShapeWithNullableField $shape): void {
  Shapes::idx($shape, 'a');
}
