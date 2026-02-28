<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function foo(): void {
  // This syntax isn't legal
  ExampleDsl<int>`1`;
}
