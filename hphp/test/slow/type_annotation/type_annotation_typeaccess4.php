<?hh

class C {
  const type T = NonExistentClass;
  const type U = this::T;
}

var_dump(type_structure(C::class, 'U'));
