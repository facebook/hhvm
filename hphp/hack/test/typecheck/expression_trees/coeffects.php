<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

async function bar(
  ExampleContext $_,
): Awaitable<ExampleExpression<ExampleFunction<(function(ExampleString): ExampleInt)>>> {
  throw new Exception();
}
function baz(): ExampleExpression<ExampleInt> {
  throw new Exception();
}

function pure()[]: void {
  // The coeffects of virtualized functions should not matter
  $fun_call = ExampleDsl`bar("baz")`;
  // But should still be respected outside the expression tree
  baz();
  // And when invoking functions when splicing
  ExampleDsl`${baz()}`;
}
