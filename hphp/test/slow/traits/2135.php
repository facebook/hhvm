<?hh

interface I {
  const FOO = "two";
}

trait T implements I {}

abstract class B implements I { use T;}

class A extends B implements I {}

<<__EntryPoint>>
function main() :mixed{
  var_dump(A::FOO);
}
