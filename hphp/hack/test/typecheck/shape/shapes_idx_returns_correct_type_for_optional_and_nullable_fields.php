<?hh // strict

type ShapeWithNullableField = shape('a' => ?int);
type ShapeWithOptionalField = shape(?'a' => int);
type ShapeWithOptionalNullableField = shape(?'a' => ?int);

function nullableFieldWithDefaultValueReturnsNonNullable(
  ShapeWithNullableField $shapeWithNullableField,
): int {
  return Shapes::idx($shapeWithNullableField, 'a', 12);
}

function optionalFieldWithDefaultValueReturnsNonNullable(
  ShapeWithOptionalField $shapeWithOptionalField,
): int {
  return Shapes::idx($shapeWithOptionalField, 'a', 12);
}

function optionalNullableFieldWithDefaultValueReturnsNonNullable(
  ShapeWithOptionalNullableField $shapeWithOptionalNullableField,
): int {
  return Shapes::idx($shapeWithOptionalNullableField, 'a', 12);
}

function nullableFieldValueReturnsNullable(
  ShapeWithOptionalField $shapeWithOptionalField,
): ?int {
  return Shapes::idx($shapeWithOptionalField, 'a');
}

function optionalFieldValueReturnsNullable(
  ShapeWithNullableField $shapeWithNullableField,
): ?int {
  return Shapes::idx($shapeWithNullableField, 'a');
}

function optionalNullableFieldValueReturnsNullable(
  ShapeWithOptionalNullableField $shapeWithOptionalNullableField,
): ?int {
  return Shapes::idx($shapeWithOptionalNullableField, 'a');
}
