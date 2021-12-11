<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function test(): void {
  $et = ExampleDsl`2 + 2`;
  hh_show($et);
}
