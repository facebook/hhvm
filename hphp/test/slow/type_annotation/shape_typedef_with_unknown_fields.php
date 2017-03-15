<?hh // strict

class Base {
  const type ShapeWithUnknownFields = shape(...);
}

$type = new ReflectionTypeConstant('Base', 'ShapeWithUnknownFields');
var_dump($type->getAssignedTypeText());
var_dump($type->getTypeStructure());
