<?hh // strict

function foo(): (function(inout int, string): int) {
  return (int $x, $y) ==> 42;
}
