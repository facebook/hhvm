<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function foo(): void {
  ExampleDsl`"abcd"`;
         //    ^ hover-at-caret
}
