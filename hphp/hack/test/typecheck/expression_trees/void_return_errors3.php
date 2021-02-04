<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function test(): void {
  Code`(ExampleBool $b): ExampleInt ==> {
    if ($b) {
      return 1;
    }
  }`;
}
