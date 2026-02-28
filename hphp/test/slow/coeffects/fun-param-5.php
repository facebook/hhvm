<?hh

class A {}
function foo(int $y, (function()[_]: void) $x)[ctx $x] :mixed{}

<<__EntryPoint>>
function main() :mixed{
  foo(0, new A);
  foo(0, new A, ...vec[]);
}
