<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function test(): void {
  ExampleDsl`(?MyState $x) ==> {
    return $x?->foo(1);
  }`;
}

abstract class MyState {
  public function foo(ExampleInt $x): void {}
}
