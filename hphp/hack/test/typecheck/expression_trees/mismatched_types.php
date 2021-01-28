<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function test(): void {
  $_ = Code`() ==> {
    $x = "hello";
    $x + 1;
  }`;
}
