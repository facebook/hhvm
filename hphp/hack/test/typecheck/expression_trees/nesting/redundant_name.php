<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function f(): void {
  $y = ExampleDsl`${ExampleDsl`1`}`; // Ok
  $y = ExampleDsl2`${ExampleDsl`1`}`; // Error: DSL name must match
  $y = ExampleDsl`${ExampleDsl2`1`}`; // Error: DSL name must match
}
