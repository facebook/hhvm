<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

// Testing to make sure lambdas still do not capture $$ and that we throw appropriate errors
function test(ExprTree<Code, Code::TAst, ExampleInt> $x): void {
  $et = $x |> (() ==> { return Code`${ $$ }`; })();
  print_et($et);
}

<<__EntryPoint>>
function entrypoint(): void {
  require 'expression_tree.inc';
  test(Code`3`);
}
