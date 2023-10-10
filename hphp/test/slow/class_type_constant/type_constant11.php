<?hh

abstract class C {
  abstract const type T;
}


<<__EntryPoint>>
function main_type_constant11() :mixed{
var_dump(C::T);
}
