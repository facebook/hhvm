<?hh

class Base {
  const type T = int;
}

interface I {
  const type U = bool;
}

class Child extends Base implements I {
}


<<__EntryPoint>>
function main_type_annotation_shape6() :mixed{
var_dump(type_structure(Child::class, 'T'));
var_dump(type_structure(Child::class, 'U'));
}
