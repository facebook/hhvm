<?hh
<<file:__EnableUnstableFeatures('expression_trees')>>

function myTestFunction1(): ExprTree<ExampleDsl, ExampleDsl::TAst, ExampleInt> {
  return ExampleDsl`2`;
}

async function myTestFunction2(): Awaitable<ExprTree<ExampleDsl, ExampleDsl::TAst, ExampleInt>> {
  return ExampleDsl`1`;
}

<<__EntryPoint>>
async function myTestFunction(): Awaitable<void> {
  require __DIR__.'/../../../hack/test/expr_tree.php';
  $x = ExampleDsl`${await myTestFunction2()}`;
  $y = ExampleDsl`1 + ${await myTestFunction2()} + ${myTestFunction1()}`;
  print_et($x);
  print_et($y);
}
