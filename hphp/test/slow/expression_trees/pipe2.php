<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function bar(ExprTree<ExampleDsl, ExampleDsl::TAst, ExampleInt> $x): ExprTree<ExampleDsl, ExampleDsl::TAst, ExampleInt> {
  return ExampleDsl`2 - ${ $x }`;
}

function combine(ExprTree<ExampleDsl, ExampleDsl::TAst, ExampleInt> $x, ExprTree<ExampleDsl, ExampleDsl::TAst, ExampleInt> $y): ExprTree<ExampleDsl, ExampleDsl::TAst, ExampleInt> {
  return ExampleDsl`${ $x } * ${ $y }`;
}

// Testing multiple pipes within the splices
function test(ExprTree<ExampleDsl, ExampleDsl::TAst, ExampleInt> $x): void {
  // (2 - (2 - (1 * (2 - 1))) * (2 - (1 * (2 - 1))))
  $et = $x |> ExampleDsl`${ combine($$, bar($$)) |> combine(bar($$), bar($$)) |> bar($$) }`;
  print_et($et);
}

<<__EntryPoint>>
function entrypoint(): void {
  require __DIR__.'/../../../hack/test/expr_tree.php';
  test(ExampleDsl`1`);
}
