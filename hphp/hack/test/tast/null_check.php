<?hh // strict

function f(?int $x): int {
  if ($x === null) return 1;
  if (null === $x) return 2;
  return 0;
}
