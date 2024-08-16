<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function foo(ExprTree<ExampleDsl, ExampleDsl::TAst, ExampleInt> $x): ExprTree<ExampleDsl, ExampleDsl::TAst, ExampleInt> {
  return ExampleDsl`${ $x } + 7`;
}

function bar(ExprTree<ExampleDsl, ExampleDsl::TAst, ExampleInt> $x): ExprTree<ExampleDsl, ExampleDsl::TAst, ExampleInt> {
  return ExampleDsl`2 - ${ $x }`;
}

function combine(ExprTree<ExampleDsl, ExampleDsl::TAst, ExampleInt> $x, ExprTree<ExampleDsl, ExampleDsl::TAst, ExampleInt> $y): ExprTree<ExampleDsl, ExampleDsl::TAst, ExampleInt> {
  return ExampleDsl`${ $x } * ${ $y }`;
}

// Testing multiple expressions using $$ in rhs of pipe
function test(ExprTree<ExampleDsl, ExampleDsl::TAst, ExampleInt> $x): void {
  // ((2 - 3) * ((100 / 3) * 3))
  $et = $x |> combine(bar($$), combine(ExampleDsl`100 / ${ $$ }`, $$));
  print_et($et);
}

<<__EntryPoint>>
function entrypoint(): void {
  require __DIR__.'/../../../hack/test/expr_tree.php';
  test(ExampleDsl`3`);
}
