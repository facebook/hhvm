<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

const int MY_CONST = 123;

function test(): void {
  $g = ExampleDsl`1 + MY_CONST`;
}
