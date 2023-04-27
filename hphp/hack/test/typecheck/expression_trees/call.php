<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

async function bar(
  ExampleContext $_,
): Awaitable<ExprTree<ExampleDsl, ExampleDsl::TAst, ExampleFunction<(function(ExampleString): ExampleInt)>>> {
  throw new Exception();
}

function foo(): void {
  $fun_call = ExampleDsl`bar("baz")`;
}
