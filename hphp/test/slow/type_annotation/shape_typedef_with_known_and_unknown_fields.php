<?hh // strict

class Base {
  const type ShapeWithKnownAndUnknownFields = shape('a' => int, ...);
}


<<__EntryPoint>>
function main_shape_typedef_with_known_and_unknown_fields() {
$type = new ReflectionTypeConstant('Base', 'ShapeWithKnownAndUnknownFields');
var_dump($type->getAssignedTypeText());
var_dump($type->getTypeStructure());
}
