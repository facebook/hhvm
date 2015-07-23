<?hh // strict

class Base {
  const type T = int;
}

interface I {
  const type U = bool;
}

class Child extends Base implements I {
}

var_dump(type_structure(Child::class, 'T'));
var_dump(type_structure(Child::class, 'U'));
