<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function test(ExprTree<Code, Code::TAst, ExampleInt> $x = Code`1`): void {}

class MyTestClass {
  public function test(
    ExprTree<Code, Code::TAst, ExampleInt> $x = Code`1`,
  ): void {}
}
