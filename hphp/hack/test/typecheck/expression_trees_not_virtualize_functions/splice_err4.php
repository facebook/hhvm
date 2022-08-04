<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function foo(bool $z): void {
  if ($z) {
    $x = ExampleDsl`4`;
  } else {
    $x = 4;
  }

  // Inferred type needs to be compatible
  $y = ExampleDsl`4 + ${$x}`;
}
