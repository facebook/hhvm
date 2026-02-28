<?hh

class Base {
  const type ShapeWithUnknownFieldsNotAtEnd = shape(
    'a' => int,
    ...,
    'b' => int,
  );
}
<<__EntryPoint>> function main(): void {
$type = new ReflectionTypeConstant('Base', 'ShapeWithUnknownFieldsNotAtEnd');
var_dump($type->getAssignedTypeText());
var_dump($type->getTypeStructure());
}
