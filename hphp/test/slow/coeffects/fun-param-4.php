<?hh

class A {
  const ctx C = [];
}

function poly(A $x)[$x::C] :mixed{}

function foo((function()[_]: void) $x)[ctx $x] :mixed{}

<<__EntryPoint>>
function main() :mixed{
  foo(poly<>);
}
