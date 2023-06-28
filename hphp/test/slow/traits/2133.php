<?hh

interface I {
  const FOO = "one";
}

interface I1 {
  const FOO = "two";
}

trait T1 implements I1 {}

trait T implements I {
  use T1;
}

class A {
  use T;
}

<<__EntryPoint>>
function main() :mixed{
  var_dump(A::FOO);
}
