<?hh

class A {
  const ctx C = [];
}

function foo(A $x)[$x::C] {
  $a = 1;
  $f = () ==> { var_dump($a); };
  $f();
}

<<__EntryPoint>>
function main() {
  foo(new A());
}
