<?hh

class A {
  const ctx C = [policied_of<A>];
}

function f(A $x)[$x::C] {}

<<__EntryPoint>>
function main() {
  f(new A);
}
