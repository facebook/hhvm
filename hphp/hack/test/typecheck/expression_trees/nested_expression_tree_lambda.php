<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function test(): void {
  Code`() ==> { $x = Code`1`; }`;
}
