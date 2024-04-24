<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>
<<file:__EnableUnstableFeatures('expression_tree_nest')>>

function f(): void {
  $x = `1`; // Error: missing DSL
  $y = ExampleDsl`${(() ==> `1`)()}`; // Error: missing DSL
}
