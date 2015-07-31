<?hh

interface I {
  const type T = this;
}

class C implements I {
}

var_dump(type_structure('I', 'T'));
var_dump(type_structure('C', 'T'));

$x = new ReflectionTypeConstant('C', 'T');
var_dump($x->getAssignedTypeText());
