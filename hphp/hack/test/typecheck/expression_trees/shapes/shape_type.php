<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function g(): ExampleExpression<ExampleFunction<(function(
    ExampleShape<shape('x' => ExampleInt)>
  ): ExampleInt)>> {
  return ExampleDsl`(ExampleShape<shape('x' => ExampleInt)> $shape) ==> 3`;
}
