<?hh

<<file: __EnableUnstableFeatures('expression_trees')>>

function test(): void {
  // ExampleFloat doesn't implement the postfix plus plus operator,
  // so don't expect this to typecheck
  ExampleDsl`() ==> {
    $i = 1.0;
    $i--;
  }`;
}
