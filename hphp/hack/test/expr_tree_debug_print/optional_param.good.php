<?hh
<<file:__EnableUnstableFeatures('expression_trees')>>

function g(): ExprTree<(function(
    optional ExampleInt,
  ): ExampleInt)> {
  return ExampleDsl`($x = 10) ==> $x`;
}
