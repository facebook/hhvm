<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

function foo_named((function(int, named string $tag): void) $x): void {
  $y = $x;
}
