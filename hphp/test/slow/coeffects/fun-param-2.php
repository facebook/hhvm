<?hh

class A {}
function foo((function()[_]: void) $x)[ctx $x] :mixed{}

<<__EntryPoint>>
function main() :mixed{
  foo(new A);
}
