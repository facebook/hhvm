<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

class Foo {
  public static function bar(): ExprTree<ExampleDsl, ExampleDsl::TAst, (function(): void)> {
    throw new Exception();
  }
}

class Quux extends Foo {}

function takes_reified<reify T as Foo>(): void {
  ExampleDsl`${ T::bar() }`;
}

function test(): void {
  takes_reified<Foo>();
  takes_reified<Quux>();
}
