<?hh

class C {
  const type T = Set<shape()>;
}


<<__EntryPoint>>
function main_type_annotation_shape2() :mixed{
$x = new ReflectionTypeConstant('C', 'T');
var_dump($x->getAssignedTypeText());
var_dump($x->getTypeStructure());
}
