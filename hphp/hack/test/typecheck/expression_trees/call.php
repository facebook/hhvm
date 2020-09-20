<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

// Placeholder definitions so we don't get naming errors.
class Code {}
function bar(mixed $_): int { return 1; }

function foo(): void {
  $fun_call = Code`bar("baz")`;
}
