<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function foo(): void {
  $assign = Code`(int $x): void ==> { $y = $x; }`;
}
