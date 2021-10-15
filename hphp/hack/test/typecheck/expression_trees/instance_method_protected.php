<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function test(ExprTree<Code, Code::TAst, string> $y): void {
  Code`(MyState $x) ==> {
    $x->foo(1);
  }`;
}

abstract class MyState {
  protected function foo(ExampleInt $x): void {}
}
