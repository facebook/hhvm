<?hh

trait T1 {
  const type FOO = int;
}

trait T2 {
  const FOO = "red";
}

class A {
  use T1, T2;
}

<<__EntryPoint>>
function main() :mixed{
  var_dump(A::FOO);
}
