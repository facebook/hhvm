<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function test(ExampleExpression<ExampleInt> $x = ExampleDsl`1`): void {}

class MyTestClass {
  public function test(
    ExampleExpression<ExampleInt> $x = ExampleDsl`1`,
  ): void {}
}
