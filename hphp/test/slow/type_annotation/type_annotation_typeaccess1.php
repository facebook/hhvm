<?hh

class C {
  const type T = int;
  const type U = C::T;
  const type V = D::T;
  const type W = D;
}

class D {
  const type T = C::U;
  const type U = C::W::V;
  const type V = bool;
}

var_dump(type_structure(C::class, 'V'));
var_dump(type_structure(D::class, 'U'));

$x = new ReflectionTypeConstant(D::class, 'U');
var_dump($x->getAssignedTypeText());
