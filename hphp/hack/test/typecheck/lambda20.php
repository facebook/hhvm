<?hh // strict

function foo(): (function(int, string, ...): int) {
  return (int $x, $y, ...) ==> $x;
}
