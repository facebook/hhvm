<?hh
<<file:__EnableUnstableFeatures('expression_trees', 'expression_tree_nested_bindings')>>

function f(): void {
  ExampleDsl`{$x = 1; $y = ${ExampleDsl`$x`}; $z = ${ExampleDsl`${ExampleDsl`$a`} + $y`};}`;
}
