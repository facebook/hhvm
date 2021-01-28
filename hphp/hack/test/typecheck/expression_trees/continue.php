<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function foo(): void {
  $n = Code`() ==> {
    while(true) {
      continue;
    }
  }`;
}
