<?hh

class A {}
function foo(int $y, (function()[_]: void) $x)[ctx $x] {}

<<__EntryPoint>>
function main() {
  foo(0, new A);
  foo(0, new A, ...vec[]);
}
