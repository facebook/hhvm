<?hh

interface I1 {
  const FOO = "2foo";
}

trait T1 implements I1 {
  const FOO = "1foo";
}

class A {
  use T1;
}

<<__EntryPoint>>
function main() :mixed{
  var_dump(A::FOO);
}
