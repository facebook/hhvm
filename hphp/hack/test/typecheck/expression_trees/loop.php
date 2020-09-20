<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

// Placeholder definitions so we don't get naming errors.
class Code {}
function bar(): void {}

function foo(): void {
  $loop = Code`($x) ==> { while(true) { bar(); } }`;
}
