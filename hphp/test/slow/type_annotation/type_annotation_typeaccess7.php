<?hh

class C {
  const type T = D::T;
}

abstract class D {
  abstract const type T;
}

var_dump(type_structure(C::class, 'T'));
