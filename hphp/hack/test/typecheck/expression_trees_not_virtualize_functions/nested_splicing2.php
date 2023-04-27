<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function test(): void {
  ExampleDsl`${1 + ${4}}`;
}
