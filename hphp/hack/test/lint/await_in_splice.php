<?hh
<<file: __EnableUnstableFeatures('expression_trees')>>
<<file: __EnableUnstableFeatures('await_in_splice')>>

async function my_test_funtion2(
): Awaitable<ExprTree<ExampleDsl, ExampleDsl::TAst, ExampleInt>> {
  return ExampleDsl`1`;
}

<<__EntryPoint>>
async function my_test_funtion(): Awaitable<void> {
  ExampleDsl`${await my_test_funtion2()}`;
}
