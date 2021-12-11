<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function foo(): void {
  $x = ExampleDsl`'Hello'`;

  // Inferred type needs to be compatible
  $y = ExampleDsl`4 + ${$x}`;
}
