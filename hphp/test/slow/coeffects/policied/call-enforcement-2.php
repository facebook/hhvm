<?hh

class A {
  const ctx C = [zoned_with<A>];
}

function f(A $x)[$x::C] :mixed{}

<<__EntryPoint>>
function main() :mixed{
  f(new A);
}
