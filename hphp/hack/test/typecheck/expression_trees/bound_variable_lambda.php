<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function test(): void {
  $_ = ExampleDsl`() ==> {
    $x = 1;
    $x + 1;
  }`;
}
