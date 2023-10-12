<?hh // strict

type ShapeWithOptionalField = shape(?'a' => int);

function foo(?ShapeWithOptionalField $shape): void {
  $shape['a'] ?? 12;
}

type ShapeWithoutOptionalField = shape('a' => int);

function bar(?ShapeWithoutOptionalField $shape): void {
  $shape['a'] ?? 12;
}
