<?hh

<<file:__EnableUnstableFeatures('expression_trees', 'expression_tree_nullsafe_obj_get')>>

<<__EntryPoint>>
function test(): void {
  require __DIR__.'/../../../hack/test/expr_tree.php';

  $et = ExampleDsl`(?MyState $x) ==> {
    return $x?->foo(1);
  }`;

  print_et($et);
}

abstract class MyState {
  public function foo(ExampleInt $x): void {}
}
