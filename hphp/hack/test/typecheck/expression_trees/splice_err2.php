<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function foo(): void {
  $x = Code`'Hello'`;

  // Inferred type needs to be compatible
  $y = Code`4 + ${$x}`;
}
