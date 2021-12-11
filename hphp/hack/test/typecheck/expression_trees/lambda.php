<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function foo(): void {
  $lambda = ExampleDsl`(mixed $x) ==> $x`;
}
