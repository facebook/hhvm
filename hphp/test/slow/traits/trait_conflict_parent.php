<?hh

trait T1 {
  const FOO = "blue";
}

class A {
  const FOO = "red";
}

class B extends A {
  use T1;
}

<<__EntryPoint>>
function main() :mixed{
  var_dump(B::FOO);
}
