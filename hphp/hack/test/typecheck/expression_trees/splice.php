<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function test(): void {
  $x = ExampleDsl`4`;
  ExampleDsl`${$x}`;
}
