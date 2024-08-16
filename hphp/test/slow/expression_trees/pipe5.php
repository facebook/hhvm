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

// Testing multiple pipes in the function
function test(ExprTree<ExampleDsl, ExampleDsl::TAst, ExampleInt> $x): void {
  // ((2 - 3) * ((100 / 3) * 3))
  $et = $x |> combine(bar($$), combine(ExampleDsl`100 / ${ $$ }`, $$));
  print_et($et);

  // ((2 - 200) * ((100 / 200) * 200))
  $et = ExampleDsl`200` |> combine(bar($$), combine(ExampleDsl`100 / ${ $$ }`, $$));
  print_et($et);

  // ((2 - 300) * ((100 / 300) * 300))
  $et = ExampleDsl`300` |> combine(bar($$), combine(ExampleDsl`100 / ${ $$ }`, $$));
  print_et($et);
}

<<__EntryPoint>>
function entrypoint(): void {
  require __DIR__.'/../../../hack/test/expr_tree.php';
  test(ExampleDsl`3`);
}
