<?hh

type ShapeWithUnknownFieldsNotAtEnd = shape(
  'a' => int,
  ...,
  'b' => int,
);
