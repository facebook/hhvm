<?hh

class C {
  const type T = int;
}

class C2 extends C {
  const type T = string;
}

function foo(C $c) :mixed{
  var_dump(type_structure($c, 'T')['classname']);
}

<<__EntryPoint>>
function main() :mixed{
  foo(new C2());
}
