<?hh

function bar($c) { return $c(1); }

class A {}

enum class B : int extends A {
  int FOO = bar($x ==> $x + 1);
}

<<__EntryPoint>>
function main() {
  var_dump(B::FOO);
}
