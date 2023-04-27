<?hh

function main(?int $x): int {
  10 ?? (($x as nonnull) + 1);
  return $x;
}
