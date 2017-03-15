<?hh // strict

class Base {
  const type ShapeWithOptionalField = shape(?'a' => int);
}

$type = new ReflectionTypeConstant('Base', 'ShapeWithOptionalField');
var_dump($type->getAssignedTypeText());
var_dump($type->getTypeStructure());
