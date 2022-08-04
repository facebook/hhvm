<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function test(): void {
  // Error, should be ExampleVoid since we virtualize void
  ExampleDsl`(): void ==> {}`;
}
