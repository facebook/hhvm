<?hh // strict

function test(bool $b, (function (int): int) $f): int {
  if ($b) {}
  return $f(1);
}
