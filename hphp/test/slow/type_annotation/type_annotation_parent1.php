<?hh

class Base {
  const type T = int;
}

class Child extends Base {
  const type T = bool;
  const type U = parent::T;
}

class GrandChild extends Child {
  const type V = parent::T;
}


<<__EntryPoint>>
function main_type_annotation_parent1() {
var_dump(type_structure(Child::class, 'U'));
var_dump(type_structure(GrandChild::class, 'U'));
var_dump(type_structure(GrandChild::class, 'V'));
}
