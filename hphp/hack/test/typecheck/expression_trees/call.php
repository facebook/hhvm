<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

async function bar(
  ExampleContext $_,
): Awaitable<ExampleExpression<ExampleFunction<(function(ExampleString): ExampleInt)>>> {
  throw new Exception();
}

function foo(): void {
  $fun_call = ExampleDsl`bar("baz")`;
}
