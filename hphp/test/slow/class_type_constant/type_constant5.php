<?hh


interface I {
  abstract const type;
}

class C implements I {
  const type = 1;
}


<<__EntryPoint>>
function main_type_constant5() :mixed{
var_dump(C::type - 1);
var_dump(C::type + 1);
}
