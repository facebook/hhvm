<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function foo(): void {
  $x = Code`4`;
  $y = Code`5`;

  // Inferred type needs to be compatible
  $z = Code`${$y} + ${$x}`;
}
