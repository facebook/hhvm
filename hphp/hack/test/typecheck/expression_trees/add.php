<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

// Placeholder definition so we don't get naming errors.
class Code {}

function foo(): void {
  $addition = Code`1 + 2`;
}
