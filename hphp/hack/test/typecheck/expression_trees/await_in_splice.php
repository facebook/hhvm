<?hh
<<file:__EnableUnstableFeatures('expression_trees')>>
<<file:__EnableUnstableFeatures('await_in_splice')>>

async function myTestFunction2(): Awaitable<ExprTree<ExampleDsl, ExampleDsl::TAst, ExampleInt>> {
  return ExampleDsl`1`;
}

<<__EntryPoint>>
async function myTestFunction(): Awaitable<void> {
  ExampleDsl`1 + ${await myTestFunction2()}`;
}
