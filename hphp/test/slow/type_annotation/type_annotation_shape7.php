<?hh // strict

abstract class Base {
  abstract const type T;
}

class Child extends Base {
  const type T = string;
}

var_dump(type_structure(Child::class, 'T'));
