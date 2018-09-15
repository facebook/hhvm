<?hh // strict

function foo(): (function(inout int): int) {
  return function(inout string $x) {
    return 42;
  };
}
