<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function foo(int $_): ExampleExpression<ExampleInt> {
  throw new Exception();
}

function test(): void {
  1 |> ExampleDsl`(): ExampleInt ==> { return ${ $$ |> foo($$) }; }`;
}
