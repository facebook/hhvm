<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function test(ExprTree<ExampleDsl, ExampleDsl::TAst, string> $y): void {
  ExampleDsl`(MyState $x) ==> {
    $x->foo(1);
  }`;
}

abstract class MyState {
  protected function foo(ExampleInt $x): void {}
}
