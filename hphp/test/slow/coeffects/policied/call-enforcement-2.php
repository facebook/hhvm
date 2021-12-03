<?hh

class A {
  const ctx C = [zoned_with<A>];
}

function f(A $x)[$x::C] {}

<<__EntryPoint>>
function main() {
  f(new A);
}
