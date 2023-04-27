<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function test(): void {
  ExampleDsl`() ==> { yield 1; }`;
}
