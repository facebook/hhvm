<?hh

interface I {
  const FOO = "one";
}

trait T1 implements I {
  const BAR = "red";
}

interface I1 {
  const FOO = "two";
}

abstract class A {
  abstract const FOO;
  abstract const BAR;
}

class C extends A implements I1 {
  use T1;
}

<<__EntryPoint>>
function main() :mixed{
  var_dump(C::FOO);
  var_dump(C::BAR);
}
