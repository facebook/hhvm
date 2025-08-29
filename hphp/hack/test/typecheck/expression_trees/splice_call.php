<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function foo(
  ExampleExpression<ExampleFunction<(function(ExampleString): ExampleInt)>> $x,
): void {
  $fun_call = ExampleDsl`(${$x})("baz")`;
}
