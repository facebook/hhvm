<?hh

newtype MyAlias = shape(
  'foo'=>NonExistentClass
);

class C {
  const type T = MyAlias;
}

var_dump(type_structure(C::class, 'T'));
