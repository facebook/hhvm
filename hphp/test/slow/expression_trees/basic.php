<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function foo(
  ExampleContext $_,
): ExprTree<MyVisitor, string, (function(string): MyVisitorInt)> {
  throw new Exception();
}

<<__EntryPoint>>
function test(): void {
  require 'expression_tree.inc';

  $et = Code`1 + foo("bar")`;

  print_et($et);
}
