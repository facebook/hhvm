<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function foo(): void {
  $assign = ExampleDsl`(int $x): ExampleVoid ==> { $y = $x; }`;
}
