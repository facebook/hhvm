<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

<<__EntryPoint>>
function test(): void {
  require __DIR__.'/../../../hack/test/expr_tree.php';

  $et = ExampleDsl`{
    1 + 2;
    true || false;
    yield 2;
    while(true) {};
  }`;
  print_et($et);
}
