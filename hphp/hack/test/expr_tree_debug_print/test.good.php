<?hh
<<file:__EnableUnstableFeatures('expression_trees')>>

function f(): void {
  $x = ExampleDsl`1`;
  ExampleDsl`${$x} + 1`;
}
