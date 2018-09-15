<?hh // strict

function foo(): (function(inout int): int) {
  return function(int $x) {
    return 42;
  };
}
