<?hh // strict

class C {
  const type T = int;
}


<<__EntryPoint>>
function main_type_constant10() {
var_dump(C::T);
}
