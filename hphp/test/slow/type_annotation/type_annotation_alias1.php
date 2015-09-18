<?hh

type MyType = int;

type MyClass = C;

class C {
  const type T = MyType;
  const type U = MyClass::T;
  const type V = ?MyType;
}

var_dump(type_structure(C::class, 'T'));
var_dump(type_structure(C::class, 'U'));
var_dump(type_structure(C::class, 'V'));
