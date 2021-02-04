<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function foo(): void {
  // This syntax isn't legal
  Code<int>`1`;
}
