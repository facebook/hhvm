<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function foo(): void {
  ExampleDsl`((ExampleInt $_): ExampleString ==> { return "Hello"; })(4)`;
}
