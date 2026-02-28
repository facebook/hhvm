<?hh

type ShapeWithOptionalField = shape('a' => ?int);

function foo(ShapeWithOptionalField $shape): void {
  $shape['a'] ?? 12;
}
