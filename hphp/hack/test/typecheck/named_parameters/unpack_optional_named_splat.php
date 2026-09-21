<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

function test(
  (function(int, optional named int $n, ...(int)): void) $f,
): void {
  $f(...tuple(0, 0));
  $f(0, ...tuple(0));
  $f(...tuple(0, 0), n = 0);
  $f(n = 0, ...tuple(0, 0));
  $f(0, 0);
  $f(0, 0, n = 0);
  $f(0, n = 0, 0);
  $f(n = 0, 0, 0);
}
