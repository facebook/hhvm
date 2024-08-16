<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

<<__EntryPoint>>
function test(): void {
  require __DIR__.'/../../../hack/test/expr_tree.php';

  $x = ExampleDsl`1`;
  $et = ExampleDsl`${ $x }`;

  print_et($et);
}
