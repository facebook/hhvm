<?hh

class A {
  const ctx C = [];
}

function foo(A $x)[$x::C] :mixed{
  $a = 1;
  $f = () ==> { var_dump($a); };
  $f();
}

<<__EntryPoint>>
function main() :mixed{
  foo(new A());
}
