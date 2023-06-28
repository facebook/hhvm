<?hh

trait T1 {
  const FOO = "blue";
}

interface I {
  const FOO = "red";
}

class A implements I {
}

class B extends A {
  use T1;
}

<<__EntryPoint>>
function main() :mixed{
  var_dump(B::FOO);
}
