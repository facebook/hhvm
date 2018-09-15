<?hh // strict

class Base {
  const type ShapeWithOptionalField = shape(?'a' => int);
}


<<__EntryPoint>>
function main_shape_typedef_with_optional_field() {
$type = new ReflectionTypeConstant('Base', 'ShapeWithOptionalField');
var_dump($type->getAssignedTypeText());
var_dump($type->getTypeStructure());
}
