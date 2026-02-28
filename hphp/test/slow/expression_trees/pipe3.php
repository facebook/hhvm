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

// Testing multiple layers of pipes
function test(ExprTree<ExampleDsl, ExampleDsl::TAst, ExampleInt> $x): void {
  /*
    ((2 - (100 + (2 - ((3 + 7) * (3 + 7))) + 100))
      * (2 - (100 + (2 - ((3 + 7) * (3 + 7))) + 100)))
  */
  $et = $x
          |> foo($$)
          |> ExampleDsl`100 + ${ combine($$, $$) |> bar($$) } + 100`
          |> bar($$)
          |> combine($$, $$);
  print_et($et);
}

<<__EntryPoint>>
function entrypoint(): void {
  require __DIR__.'/../../../hack/test/expr_tree.php';
  test(ExampleDsl`3`);
}
