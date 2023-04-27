<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function test(): void {
  ExampleDsl`() ==> {
    $x = ${ ExampleDsl`1`};
  }`;
}
