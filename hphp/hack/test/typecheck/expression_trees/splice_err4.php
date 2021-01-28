<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function foo(bool $z): void {
  if ($z) {
    $x = Code`4`;
  } else {
    $x = 4;
  }

  // Inferred type needs to be compatible
  $y = Code`4 + ${$x}`;
}
