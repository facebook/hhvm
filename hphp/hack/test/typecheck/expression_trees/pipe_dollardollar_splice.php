<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function test(ExampleExpression<ExampleInt> $x): void {
  $x |> ExampleDsl`${ $$ }`;
}
