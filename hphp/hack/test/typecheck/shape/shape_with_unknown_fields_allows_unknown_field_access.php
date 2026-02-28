<?hh

type ShapeWithUnknownFields = shape(...);

function f(ShapeWithUnknownFields $shape): void {
  Shapes::idx($shape, 'x');
}
