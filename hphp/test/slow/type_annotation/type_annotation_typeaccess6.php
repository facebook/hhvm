<?hh

class C {
  const type T = C::TClass::T;
  const type TClass = D;
}

class D {
}

var_dump(type_structure(C::class, 'T'));
