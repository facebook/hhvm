<?hh
<<file:__EnableUnstableFeatures('expression_trees')>>
<<file:__EnableUnstableFeatures('expression_tree_nested_bindings')>>

function f(): void {
  $a = ExampleDsl`{$x = 1; return ${ExampleDsl`$x`};}`;
  ExampleDsl`${$a} + 1`;
  $a = ExampleDsl`{$x = 1; return ${ExampleDsl`1+$x`};}`;
  ExampleDsl`${$a} + 1`;
}
