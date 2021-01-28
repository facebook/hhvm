<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function test(): void {
  $x = 1;

  // Expression Trees do not inherit local variables from the outer scope
  $_ = Code`$x + 1`;
}
