<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function foo(): void {
  $assign = Code`(int $x): ExampleVoid ==> { $y = $x; }`;
}
