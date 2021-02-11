<?hh

trait T1 {
  const FOO = "blue";
}

class A {
  use T1;
  const FOO = "red";
}

<<__EntryPoint>>
function main() {
  var_dump(A::FOO);
}
