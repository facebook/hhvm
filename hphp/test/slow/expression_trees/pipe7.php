<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

// Testing to make sure lambdas bodies don't capture $$ but call arguments do
function test(ExprTree<ExampleDsl, ExampleDsl::TAst, ExampleInt> $x): void {
  // 3
  $et = $x |> ExampleDsl`${ (($x) ==> { return $x; })($$) }`;

  print_et($et);
}

<<__EntryPoint>>
function entrypoint(): void {
  require __DIR__.'/../../../hack/test/expr_tree.php';
  test(ExampleDsl`3`);
}
