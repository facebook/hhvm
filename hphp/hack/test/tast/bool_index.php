<?hh

function f<T>(mixed $m, T $t, nonnull $n): void {
  $m[true]; // The key type argument to expected type of $m's hole is `nothing`
  $m[$t]; // The key type argument to expected type of $m's hole is `T & arraykey`
  $m[$n]; // The key type argument to expected type of $m's hole is `arraykey`
  $m[$m]; // The key type argument to expected type of $m's hole is `arraykey`
}
