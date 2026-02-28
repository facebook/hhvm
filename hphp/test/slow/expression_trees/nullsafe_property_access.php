<?hh

<<file:__EnableUnstableFeatures('expression_trees', 'expression_tree_nullsafe_obj_get')>>

<<__EntryPoint>>
function test(): void {
  require __DIR__.'/../../../hack/test/expr_tree.php';

  $et = ExampleDsl`(?MyState $x): ?ExampleInt ==> {
    return $x?->my_prop;
  }`;

  print_et($et);
}

abstract class MyState {
  public ExampleInt $my_prop;
}
