<?hh
<<file: __EnableUnstableFeatures('expression_trees')>>

async function myTestFunction2(
): Awaitable<ExprTree<ExampleDsl, ExampleDsl::TAst, ExampleInt>> {
  echo 1;
  await RescheduleWaitHandle::create(0, 0);
  echo 2;
  return ExampleDsl`2`;
}

<<__EntryPoint>>
async function myTestFunction(): Awaitable<void> {
  require __DIR__.'/../../../hack/test/expr_tree.php';
  $y = ExampleDsl`
    ${await myTestFunction2()} +
    ${await myTestFunction2()}
  `;
}
