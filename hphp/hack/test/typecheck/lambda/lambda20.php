<?hh // strict

function foo(): (function(int, string, mixed...): int) {
  return (int $x, $y, ...$args) ==> $x;
}
