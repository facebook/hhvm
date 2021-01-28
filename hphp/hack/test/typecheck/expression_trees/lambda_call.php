<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function foo(): void {
  Code`((ExampleInt $_): ExampleString ==> { return "Hello"; })(4)`;
}
