<?hh

class A {}
function foo((function()[_]: void) $x)[ctx $x] :mixed{}

<<__EntryPoint>>
function main() :mixed{
  __hhvm_intrinsics\launder_value(foo<>)(new A);
}
