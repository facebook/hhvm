<?hh
<<file:__EnableUnstableFeatures('expression_trees', 'expression_tree_nullsafe_obj_get')>>

function f(): void {
  ExampleDsl`(?MyState $x) ==> {
    return $x?->foo(1);
  }`;
}

abstract class MyState {
  public function foo(ExampleInt $x): void {}
}
