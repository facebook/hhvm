<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function foo(): void {
  Code`"abcd"`;
// ^ hover-at-caret
}
