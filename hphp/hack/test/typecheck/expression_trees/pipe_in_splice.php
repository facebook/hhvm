<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function foo(int $_): ExampleExpression<ExampleInt> {
  throw new Exception();
}

function test(): void {
  ExampleDsl`(): ExampleInt ==> { return ${ 1 |> foo($$) }; }`;
}
