<?hh
<<file:__EnableUnstableFeatures('expression_trees')>>
<<file:__EnableUnstableFeatures('expression_tree_nested_bindings')>>

function f(): void {
  ExampleDsl`${ExampleDsl`$x`}`;
}
