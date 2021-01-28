<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function test(): void {
  $et = Code`2 + 2`;
  hh_show($et);
}
