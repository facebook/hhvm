<?hh

class C {
  const type T = int;
}

class C2 extends C {
  const type T = string;
}

function foo(C $c) {
  var_dump(type_structure($c, 'T')['classname']);
}

<<__EntryPoint>>
function main() {
  foo(new C2());
}
