<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function foo(): void {
  $x = 4;

  // Spliced Expressions need to be ExprTrees
  $y = Code`${$x}`;
}
