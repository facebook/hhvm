<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function foo(): void {
  $one = ExampleDsl`1`;

  $one_splice = ExampleDsl`${$one} + 1`;
  //                                 ^ hover-at-caret
}
