<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

function test(
  (function(int, optional named int $n, ...(int)): void) $f,
): void {
  $f(...tuple(0));
  $f(...tuple(0, 0, 0));
  $f(...tuple('wrong', 0));
  $f(0, 'wrong', n = 0);
}

function test_distinct_types(
  (function(int, optional named string $n, ...(bool)): void) $f,
): void {
  $f(n = 'name', 0, 'wrong');
  $f(0, n = 'name', 'wrong');
  $f(0, 'wrong', n = 'name');
}
//
