<?hh

interface I1 {
  const FOO = "blue"; // should silently lose
}

trait T1 implements I1 {
}

interface I {
  const FOO = "red";
}

class A implements I {
  use T1;
}

<<__EntryPoint>>
function main() :mixed{
  var_dump(A::FOO);
}
