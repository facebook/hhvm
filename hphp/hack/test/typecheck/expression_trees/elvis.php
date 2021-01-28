<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function test(): void {
  // Currently no support for Elvis operator
  $_ = Code`true ?: "Hello"`;
}
