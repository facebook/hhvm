<?hh

class A {}

function f(A $x)[$x::C] { echo "in f\n"; }

<<__EntryPoint>>
function main() {
  f(new A);
}
