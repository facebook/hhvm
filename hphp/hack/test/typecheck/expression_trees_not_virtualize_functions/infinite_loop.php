<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function foo(): void {
  $infinite_for =
    ExampleDsl`() ==> {
      for (;;) {}
    }`;
}
