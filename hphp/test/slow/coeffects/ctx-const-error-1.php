<?hh

class A {}

function f(A $x)[$x::C] :mixed{ echo "in f\n"; }

<<__EntryPoint>>
function main() :mixed{
  f(new A);
}
