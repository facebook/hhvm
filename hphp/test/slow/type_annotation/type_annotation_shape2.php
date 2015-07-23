<?hh // strict

class C {
  const type T = foo<shape()>;
}

$x = new ReflectionTypeConstant('C', 'T');
var_dump($x->getAssignedTypeText());
var_dump($x->getTypeStructure());
