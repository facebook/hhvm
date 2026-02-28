<?hh

interface I1 {
  const type T = int;
}

trait T1 implements I1 {}

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
