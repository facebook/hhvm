<?hh // strict

function f(?int $x): int {
  $x ??= 0;
  return $x;
}
