<?hh // strict

class Base {
  const type ShapeWithKnownAndUnknownFields = shape('a' => int, ...);
}

$type = new ReflectionTypeConstant('Base', 'ShapeWithKnownAndUnknownFields');
var_dump($type->getAssignedTypeText());
var_dump($type->getTypeStructure());
