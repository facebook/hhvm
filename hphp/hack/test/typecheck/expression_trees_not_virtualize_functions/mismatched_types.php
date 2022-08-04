<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function test(): void {
  $_ = ExampleDsl`() ==> {
    $x = "hello";
    $x + 1;
  }`;
}
