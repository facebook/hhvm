<?hh

interface I {
  const FOO = "one";
}

trait T implements I {}

class B {
  use T;
}

class A extends B implements I {
}

<<__EntryPoint>>
function main() :mixed{
  var_dump(A::FOO);
}
