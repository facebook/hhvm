<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

final class MyClass {
}

<<__EntryPoint>>
function test(): void {
  require __DIR__.'/../../../hack/test/expr_tree.php';

  $et = ExampleDsl`MyClass::class`;

  print_et($et);
}
