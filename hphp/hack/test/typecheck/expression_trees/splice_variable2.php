<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function test(): void {
  $x = 1;

  // Expression Trees do not inherit local variables from the outer scope
  $_ = Code`3`;

  // But the environment afterwards should contain all of the local variables
  $x + 1;
}
