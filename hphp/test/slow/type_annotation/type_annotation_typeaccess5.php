<?hh

class C {
  const type T = this;
  const type U = NonExistentClass;
  const type V = C::T::U::T;
}

var_dump(type_structure(C::class, 'V'));
