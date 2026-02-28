<?hh
<<file:__EnableUnstableFeatures('expression_trees')>>
<<file:__EnableUnstableFeatures('expression_tree_nested_bindings')>>

function f(): void {
  ExampleDsl`{$x = ""; return ${ExampleDsl`1 + $x`};}`; // string/int mismatch
  ExampleDsl`{$x = 1; return ${ExampleDsl`$x + 1`};}`;  // Inference is too weak
}
