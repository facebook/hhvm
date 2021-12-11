<?hh

<<file: __EnableUnstableFeatures('expression_trees')>>

function test(): void {
  // This doesn't work in Hack proper. Just making sure this doesn't work here
  ExampleDsl`1--`;
}
