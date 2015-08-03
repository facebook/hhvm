<?hh

class C {
  const type T = NonExistentAlias;
}

var_dump(type_structure(C::class, 'T'));
