<?hh

trait T1 {
  abstract const type T = int;
}

interface I {
  const type T = string;
}

class C implements I {
  use T1;
}

<<__EntryPoint>>
function main() :mixed{
  var_dump(type_structure(C::class, 'T'));
}
