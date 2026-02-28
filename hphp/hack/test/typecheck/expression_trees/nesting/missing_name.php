<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function f(): void {
  $x = `1`; // Error: missing DSL
  $y = ExampleDsl`${(() ==> `1`)()}`; // Error: missing DSL
}
