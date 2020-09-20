<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

// Placeholder definition so we don't get naming errors.
class Code {}

function foo(): void {
  $lambda = Code`($x) ==> $x`;
}
