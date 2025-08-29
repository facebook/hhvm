<?hh
<<file:__EnableUnstableFeatures('expression_trees')>>

async function myTestFunction2(): Awaitable<ExampleExpression<ExampleInt>> {
  return ExampleDsl`1`;
}

<<__EntryPoint>>
async function myTestFunction(): Awaitable<void> {
  ExampleDsl`1 + ${await myTestFunction2()}`;
}
