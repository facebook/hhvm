<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function test(ExprTree<ExampleDsl, ExampleDsl::TAst, ExampleInt> $x = ExampleDsl`1`): void {}

class MyTestClass {
  public function test(
    ExprTree<ExampleDsl, ExampleDsl::TAst, ExampleInt> $x = ExampleDsl`1`,
  ): void {}
}
