<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function test(): void {
  $_ = Code`() ==> {
    $x = 1;
    $x + 1;
  }`;
}
