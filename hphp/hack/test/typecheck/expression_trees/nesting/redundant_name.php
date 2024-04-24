<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>
<<file:__EnableUnstableFeatures('expression_tree_nest')>>

function f(): void {
  $y = ExampleDsl`${ExampleDsl`1`}`; // Error: DSL name should not be in nest
  $y = ExampleDsl2`${ExampleDsl`1`}`; // Error: DSL name should not be in nest
  $y = ExampleDsl`${ExampleDsl2`1`}`; // Error: DSL name should not be in nest
}
