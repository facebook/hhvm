<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

<<__EntryPoint>>
function test(): void {
  require __DIR__.'/../../../hack/test/expr_tree.php';

  print_et(ExampleDsl`5`);
  print_et(ExampleDsl`"Hello"`);
  print_et(ExampleDsl`3.14`);
  print_et(ExampleDsl`false`);
  print_et(ExampleDsl`null`);
  print_et(ExampleDsl`() ==> {}`);
  print_et(ExampleDsl`() ==> { return; }`);
}
