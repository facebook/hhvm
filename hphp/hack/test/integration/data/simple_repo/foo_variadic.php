<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

function foo_variadic(
  (function(int, string...): void) $x,
  (function(int, named bool...): void) $y,
): void {
  $a = $x;
  $b = $y;
}
