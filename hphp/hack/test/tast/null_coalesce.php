<?hh // strict

function f(?int $x): int {
  return $x ?? 0;
}
