<?hh

function f<T>(mixed $m, nonnull $n, T $t): void {
  $m[0]; // Hole actual type should be `mixed`
  $n[0]; // Hole actual type should be `nonnull`
  $t[0]; // Hole actual type should be `T`
}
