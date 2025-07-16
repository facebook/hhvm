<?hh
<<file:__EnableUnstableFeatures('expression_trees', 'expression_tree_nullsafe_obj_get')>>

function f(): void {
  ExampleDsl`(?MyState $x): ?ExampleInt ==> {
    return $x?->my_prop;
  }`;
}

abstract class MyState {
  public ExampleInt $my_prop;
}
