<?hh // strict

abstract class C {
  abstract const type T;
}


<<__EntryPoint>>
function main_type_constant11() {
var_dump(C::T);
}
