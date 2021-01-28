<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function test(): void {
  $_ = Code`true ? 1 : "Hello"`;
}
