<?hh

<<file: __EnableUnstableFeatures('expression_trees', 'expression_tree_nullsafe_obj_get')>>

function test_error(): void {
  ExampleDsl`(?MyState $x) ==> {
    // Expected error: $x is nullable, $x?->my_prop is also nullable
    return 1 + $x?->my_prop;
  }`;
}

abstract class MyState {
  public ExampleInt $my_prop;
}
