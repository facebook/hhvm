<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function g(): ExampleExpression<
  ExampleFunction<(function(optional ExampleInt): ExampleInt)>,
> {
  return ExampleDsl`($x = 10) ==> $x`;
}
