<?hh // strict

class D {
  const int F1 = 42;
  const string F2 = 'bar';
}

class C {
  const type T = shape(D::F1=>int, D::F2=>string);
}

$x = new ReflectionTypeConstant('C', 'T');
var_dump($x->getAssignedTypeText());
var_dump($x->getTypeStructure());
