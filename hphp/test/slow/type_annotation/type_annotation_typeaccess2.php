<?hh

class C {
  const type T = int;
  const type Tthis = this;
  const type TClass = C;
  const type U = C::Tthis::TClass::T;
  const type V = this::TClass::T;
}

class D extends C {
  const type T = float;
  const type TClass = E;
}

class E {
  const type T = bool;
}

var_dump(type_structure(C::class, 'U'));
var_dump(type_structure(C::class, 'V'));
var_dump(type_structure(D::class, 'U'));
var_dump(type_structure(D::class, 'V'));
