<?hh

type MyAlias = int;
type MyAlias1 = MyAlias;
type MyAlias2 = MyAlias1;

class C {
  const type T = MyAlias2::T;
}

var_dump(type_structure(C::class, 'T'));
