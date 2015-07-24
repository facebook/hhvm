<?hh // strict

class C {
  const type T = D;
  const type U = C;
}

class D {
  const type T = C;
}

var_dump(type_structure(C::class, 'T'));
var_dump(type_structure(C::class, 'U'));
var_dump(type_structure(D::class, 'T'));
