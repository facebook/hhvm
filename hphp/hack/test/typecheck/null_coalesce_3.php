<?hh

function f(
  nonnull $nn,
  null $n,
  dict<string, nonnull> $dnn,
  dict<string, null> $dn,
): void {
  if ($nn is null) {}

  $x = $nn ?? 4;

  if ($n is null) {}

  $y = $n ?? 5;

  $_ok = $dnn['a'] ?? 6;

  $_ok = $dn['a'] ?? 7;
}
