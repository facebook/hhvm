<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function test(): void {
  ExampleDsl`(ExampleBool $b): ExampleInt ==> {
    if ($b) {
      return 1;
    }
  }`;
}
