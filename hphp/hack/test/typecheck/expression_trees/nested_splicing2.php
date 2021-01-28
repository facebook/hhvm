<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function test(): void {
  Code`${1 + ${4}}`;
}
