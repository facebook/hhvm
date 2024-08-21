<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function g(): ExprTree<ExampleDsl,
ExampleDsl::TAst, ExampleFunction<(function(
  optional ExampleInt
): ExampleInt)>> {
  return ExampleDsl`($x = 10) ==> $x`;
}
