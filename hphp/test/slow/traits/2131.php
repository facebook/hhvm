<?hh

trait T {
  const FOO = "two";
}

class B {
  use T;
}

interface I {
  const FOO = "one";
}

class A extends B implements I {}

<<__EntryPoint>>
function main() :mixed{
  var_dump(A::FOO);
}
