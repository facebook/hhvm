<?hh

interface I2 {
  const FOO = "2foo";
}

interface I extends I2 {
  const FOO = "1foo";
}

trait T1 implements I {}

class A {
  use T1;
}

<<__EntryPoint>>
function main() :mixed{
  var_dump(A::FOO);
}
