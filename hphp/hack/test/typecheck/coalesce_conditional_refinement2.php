<?hh

function main(?int $x): int {
  $y = 10;
  $y ??= ($x as nonnull) + 1;
  return $x;
}
