<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function test(): void {
  // Currently no support for Elvis operator
  $_ = ExampleDsl`true ?: "Hello"`;
}
