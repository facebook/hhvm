<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function foo(): void {
  // I am a comment associated with $one.
  $one = Code`1`;

  $one_splice = Code`${$one}`;
  //                    ^ hover-at-caret
}
