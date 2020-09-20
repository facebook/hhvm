<?hh // strict

/* This test verifies our ability to typecheck the body of a lambda when it is
 * used as an argument to a generically-typed higher-order function. */


function array_mapp<T, TTo>((function(T): TTo) $f, array<T> $xs): TTo {
  return $f($xs[0]);
}

function rev_array_mapp<T, TTo>(array<T> $xs, (function(T): TTo) $f): TTo {
  return $f($xs[0]);
}

function f(array<string> $a): void {
  array_mapp($x ==> $x + 1, $a);
  rev_array_mapp($a, $x ==> $x + 1);
}
