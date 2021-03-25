<?hh

class A {}
function foo((function()[_]: void) $x)[ctx $x] {}

<<__EntryPoint>>
function main() {
  foo(new A);
}
