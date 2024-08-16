<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function foo(
  ExampleContext $_,
): ExprTree<MyVisitor, string, (function(string): MyVisitorInt)> {
  throw new Exception();
}

<<__EntryPoint>>
function test(): void {
  require __DIR__.'/../../../hack/test/expr_tree.php';

  $et = ExampleDsl`mapType { 1 => 2, 3 => 4 }`;

  print_et($et);
}
