<?hh
<<file:__EnableUnstableFeatures('expression_trees')>>

function g(): ExprTree<(function(
    ExampleInt,
    optional ExampleInt,
  ): ExampleInt)> {
  return ExampleDsl`($_, $x = 10) ==> $x`;
}
