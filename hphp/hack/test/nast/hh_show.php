<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function test(): void {
  Code`() ==> {
    $x = 1;
    hh_show($x);
  }`;
}
