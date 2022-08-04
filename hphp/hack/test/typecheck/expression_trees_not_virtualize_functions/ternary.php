<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function test(): void {
  $_ = ExampleDsl`true ? 1 : "Hello"`;
}
