<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>
<<file: __EnableUnstableFeatures('expression_tree_array')>>

function foo(): void {
  ExampleDsl`ExampleKeyedCollection { "123" => 1, "456" => 2 }`;
                                  //    ^ hover-at-caret
}
