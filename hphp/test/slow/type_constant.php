<?hh

trait T1 {
  const type T = string;
}

class C {
  use T1;
}

<<__EntryPoint>>
function main() {
  var_dump(type_structure(C::class, 'T'));
}
