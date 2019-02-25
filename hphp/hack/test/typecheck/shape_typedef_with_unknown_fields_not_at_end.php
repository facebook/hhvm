<?hh // partial

type ShapeWithUnknownFieldsNotAtEnd = shape(
  'a' => int,
  ...,
  'b' => int,
);
