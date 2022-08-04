<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function foo(): void {
  $neg_int = ExampleDsl`-1`;
}
