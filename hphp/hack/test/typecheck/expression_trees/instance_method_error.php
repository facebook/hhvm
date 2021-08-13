<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function test(): void {
  Code`(MyState $x) ==> {
    return $x->foo(1);
  }`;
}

abstract class MyState {
  public function foo(ExampleInt): void {}
}
