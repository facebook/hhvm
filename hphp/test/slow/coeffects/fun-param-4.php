<?hh

class A {
  const ctx C = [];
}

function poly(A $x)[$x::C] {}

function foo((function()[_]: void) $x)[ctx $x] {}

<<__EntryPoint>>
function main() {
  foo(poly<>);
}
