<?hh

function foo((function()[_]: void) $x)[ctx $x] {}

<<__EntryPoint>>
function main() {
  foo(1);
}
