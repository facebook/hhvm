<?hh // strict

class C {
  const type T = NonExistentClass;
}

var_dump(type_structure(C::class, 'T'));
