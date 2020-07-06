<?hh

class A {}

class B {}

final class Test {
  public static function foo<reify T as A>(): void {
    var_dump(HH\ReifiedGenerics\get_type_structure<T>());
  }
}

<<__EntryPoint>>
function main(): void {
  // Don't enforce type constraints on reified generics
  $f = Test::foo<B>;
  $f();

  Test::foo<B>();
}
