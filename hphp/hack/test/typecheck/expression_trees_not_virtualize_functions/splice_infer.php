<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function foo(): void {
  $x = ExampleDsl`4`;
  $y = ExampleDsl`5`;

  // Inferred type needs to be compatible
  $z = ExampleDsl`${$y} + ${$x}`;
}
