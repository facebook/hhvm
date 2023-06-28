<?hh

interface I2 {
  const FOO = "two";
}

trait T implements I2 {}

class B {
  use T;
}

interface I {
  const FOO = "one";
}

class A extends B implements I {

}

<<__EntryPoint>>
function main() :mixed{
  var_dump(A::FOO);
}
