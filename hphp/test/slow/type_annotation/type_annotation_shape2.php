<?hh // strict

class C {
  const type T = Set<shape()>;
}

$x = new ReflectionTypeConstant('C', 'T');
var_dump($x->getAssignedTypeText());
var_dump($x->getTypeStructure());
