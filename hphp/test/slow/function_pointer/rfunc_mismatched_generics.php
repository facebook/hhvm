<?hh

class A {}

class B {}

function foo<reify T as A>(): void {
  var_dump(HH\ReifiedGenerics\get_type_structure<T>());
}

<<__EntryPoint>>
function main(): void {
  // We don't enforce type constraints on reified generics
  $f = foo<B>;
  $f();

  foo<B>();
}
