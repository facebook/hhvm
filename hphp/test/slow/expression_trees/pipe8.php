<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

// Testing to make sure lambdas bodies don't capture $$ and we throw appropriate errors
function test(ExprTree<Code, Code::TAst, ExampleInt> $x): void {
  $et = $x |> Code`${ (() ==> { return $$; })() }`;

  print_et($et);
}

<<__EntryPoint>>
function entrypoint(): void {
  require 'expression_tree.inc';
  test(Code`3`);
}
