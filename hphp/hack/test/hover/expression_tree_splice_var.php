<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function foo(): void {
  // I am a comment associated with $one.
  $one = ExampleDsl`1`;

  $one_splice = ExampleDsl`${$one}`;
  //                           ^ hover-at-caret
}
