<?hh

<<file:__EnableUnstableFeatures('expression_trees', 'capture_pipe_variables')>>

// Testing to make sure lambdas still do not capture $$ and that we throw appropriate errors
function test(ExprTree<ExampleDsl, ExampleDsl::TAst, ExampleInt> $x): void {
  $et = $x |> (() ==> { return ExampleDsl`${ $$ }`; })();
  print_et($et);
}

<<__EntryPoint>>
function entrypoint(): void {
  require __DIR__.'/../../../hack/test/expr_tree.php';
  test(ExampleDsl`3`);
}
