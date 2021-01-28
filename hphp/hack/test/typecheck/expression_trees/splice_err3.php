<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function foo(bool $z): void {
  if ($z) {
    $x = Code`'Hello'`;
  } else {
    $x = Code`4`;
  }

  // Inferred type needs to be compatible
  $y = Code`4 + ${$x}`;
}
