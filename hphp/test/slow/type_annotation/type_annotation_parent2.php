<?hh

class C {
  const type T = parent;
}

var_dump(type_structure(C::class,'T'));
