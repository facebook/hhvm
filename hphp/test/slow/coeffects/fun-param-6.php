<?hh

class A {}
function foo((function()[_]: void) $x)[ctx $x] {}

<<__EntryPoint>>
function main() {
  __hhvm_intrinsics\launder_value(foo<>)(new A);
}
