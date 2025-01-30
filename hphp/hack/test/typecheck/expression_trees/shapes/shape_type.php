<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function g(): ExampleDslExpression<ExampleFunction<(function(
    shape('x' => ExampleInt)
  ): ExampleInt)>> {
  return ExampleDsl`(shape('x' => ExampleInt) $shape) ==> 3`;
}
