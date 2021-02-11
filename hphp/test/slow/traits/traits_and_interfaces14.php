<?hh

interface I1 {
  const FOO = "1foo";
}

interface I2 {
  const FOO = "2foo";
}

trait T1 implements I1, I2 {}

class A {
  use T1;
}

<<__EntryPoint>>
function main() {
  var_dump(A::FOO);
}
