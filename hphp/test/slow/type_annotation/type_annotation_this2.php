<?hh

interface I {
  const type T = this;
}

class C implements I {
}

var_dump(type_structure(I::class, 'T'));
var_dump(type_structure(C::class, 'T'));

$x = new ReflectionTypeConstant(C::class, 'T');
var_dump($x->getAssignedTypeText());
