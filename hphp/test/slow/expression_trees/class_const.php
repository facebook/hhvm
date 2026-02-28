<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

final class MyClass {
  const FOO = "BAR";
}

<<__EntryPoint>>
function test(): void {
  require __DIR__.'/../../../hack/test/expr_tree.php';

  $et = ExampleDsl`MyClass::FOO`;

  print_et($et);
}
