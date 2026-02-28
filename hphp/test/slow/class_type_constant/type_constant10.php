<?hh

class C {
  const type T = int;
}


<<__EntryPoint>>
function main_type_constant10() :mixed{
var_dump(C::T);
}
