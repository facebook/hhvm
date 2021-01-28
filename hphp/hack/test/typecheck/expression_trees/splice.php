<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function test(): void {
  $x = Code`4`;
  Code`${$x}`;
}
