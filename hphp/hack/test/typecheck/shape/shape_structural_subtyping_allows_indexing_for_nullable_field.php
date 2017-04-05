<?hh // strict

type ShapeWithNullableField = shape('a' => ?int);

function foo(ShapeWithNullableField $shape): void {
  $shape['a'];
}
