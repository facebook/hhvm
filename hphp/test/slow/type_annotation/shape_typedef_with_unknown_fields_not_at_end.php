<?hh // strict

class Base {
  const type ShapeWithUnknownFieldsNotAtEnd = shape(
    'a' => int,
    ...,
    'b' => int,
  );
}

$type = new ReflectionTypeConstant('Base', 'ShapeWithUnknownFieldsNotAtEnd');
var_dump($type->getAssignedTypeText());
var_dump($type->getTypeStructure());
