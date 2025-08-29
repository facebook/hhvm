<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function foo(int $_): ExampleExpression<ExampleInt> {
  throw new Exception();
}

function bar<T>(ExampleExpression<T> $x): ExampleExpression<T> {
  return $x;
}

function test(ExampleExpression<ExampleInt> $x): void {
  $x |> ExampleDsl`${ $$ } + ${ $$ |> bar($$) } + ${ 1 |> foo($$) }`;
}
