<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function foo(): void {
  $n = ExampleDsl`true && false`;
}
