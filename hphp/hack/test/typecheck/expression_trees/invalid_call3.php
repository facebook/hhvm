<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function foo(): void {
  ExampleDsl`(2 + 2)(4)`;
}
