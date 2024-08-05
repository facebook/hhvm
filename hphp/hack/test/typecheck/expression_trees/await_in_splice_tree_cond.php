<?hh
<<file: __EnableUnstableFeatures('expression_trees')>>
<<file: __EnableUnstableFeatures('await_in_splice')>>

async function myTestFunction1(): Awaitable<ExprTree<ExampleDsl, ExampleDsl::TAst, ExampleInt>> {
  return ExampleDsl`2`;
}

<<__EntryPoint>>
async function myTestFunction(): Awaitable<void> {
  ExampleDsl`(ExampleBool $b) ==> $b ? ${await myTestFunction1()} : 1`;
}
