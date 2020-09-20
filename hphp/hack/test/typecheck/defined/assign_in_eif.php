<?hh // strict

function f(bool $b): int {
  ($b ? $x = 1 : $x = 0);
  return $x;
}
