<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function foo(): void {
  $n = ExampleDsl`() ==> {
    while(true) {
      break;
    }
  }`;
}
