<?hh

<<file:__EnableUnstableFeatures('expression_trees', 'capture_pipe_variables')>>

// Testing to make sure lambdas bodies don't capture $$ and we throw appropriate errors
function test(ExprTree<ExampleDsl, ExampleDsl::TAst, ExampleInt> $x): void {
  $et = $x |> ExampleDsl`${ (() ==> { return $$; })() }`;

  print_et($et);
}

<<__EntryPoint>>
function entrypoint(): void {
  require __DIR__.'/../../../hack/test/expr_tree.php';
  test(ExampleDsl`3`);
}
