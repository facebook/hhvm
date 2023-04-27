<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function foo(): void {
  ExampleDsl`1 + 2`;
  ExampleDsl`1 - 2`;
  ExampleDsl`1 * 2`;
  ExampleDsl`1 / 2`;
}
