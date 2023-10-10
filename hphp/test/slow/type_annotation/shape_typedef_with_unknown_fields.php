<?hh

class Base {
  const type ShapeWithUnknownFields = shape(...);
}


<<__EntryPoint>>
function main_shape_typedef_with_unknown_fields() :mixed{
$type = new ReflectionTypeConstant('Base', 'ShapeWithUnknownFields');
var_dump($type->getAssignedTypeText());
var_dump($type->getTypeStructure());
}
