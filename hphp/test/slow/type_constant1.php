<?hh

trait T1 {
  const type T = int;
}

class B {
  const type T = string;
}

class C extends B {
  use T1;
}

<<__EntryPoint>>
function main() :mixed{
  var_dump(type_structure(C::class, 'T'));
}
