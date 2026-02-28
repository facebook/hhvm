<?hh

function f0(?int $x): int {
  $x ??= 0;
  return $x;
}
