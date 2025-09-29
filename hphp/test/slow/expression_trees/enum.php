<?hh

<<file: __EnableUnstableFeatures('expression_trees')>>

enum MyEnum: int {
  FOO = 123;
}

<<__EntryPoint>>
function test(): void {
  require __DIR__.'/../../../hack/test/expr_tree.php';

  $et = ExampleDsl`MyEnum::FOO`;

  print_et($et);
}
