<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

<<__EntryPoint>>
function test(): void {
  require __DIR__.'/../../../hack/test/expr_tree.php';

  $et = ExampleDsl`{}`;
  print_et($et);

  $et = ExampleDsl`{
    1 + 2;
    true || false;
    while(true) {};
  }`;
  print_et($et);
}
