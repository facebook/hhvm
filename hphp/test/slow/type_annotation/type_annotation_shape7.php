<?hh // strict

abstract class Base {
  abstract const type T;
}

class Child extends Base {
  const type T = string;
}


<<__EntryPoint>>
function main_type_annotation_shape7() {
var_dump(type_structure(Child::class, 'T'));
}
