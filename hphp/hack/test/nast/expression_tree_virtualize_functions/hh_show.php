<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function test(): void {
  ExampleDsl`() ==> {
    $x = 1;
    hh_show($x);
  }`;
}
