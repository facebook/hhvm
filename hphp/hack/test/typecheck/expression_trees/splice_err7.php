<?hh
<<file:__EnableUnstableFeatures('expression_trees')>>

function f(): void {
  ExampleDsl`${$x ==> ${$x}}`;
}
