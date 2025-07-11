<?hh
<<file:__EnableUnstableFeatures('expression_trees')>>

<<__EntryPoint>>
function test(): void {
  require __DIR__.'/../../../hack/test/expr_tree.php';

  print_et(ExampleDsl`(?ExampleInt $x) ==> $x is null`);
  print_et(ExampleDsl`(?ExampleInt $x) ==> $x is nonnull`);
  print_et(ExampleDsl`(?ExampleInt $x) ==> $x is ExampleInt`);
}
