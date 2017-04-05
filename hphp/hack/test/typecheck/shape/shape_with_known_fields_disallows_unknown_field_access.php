<?hh // strict

type ShapeWithKnownFields = shape();

function f(ShapeWithKnownFields $shape): void {
  Shapes::idx($shape, 'x');
}
