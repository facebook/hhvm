<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function foo(): void {
  $lambda = Code`(mixed $x) ==> $x`;
}
