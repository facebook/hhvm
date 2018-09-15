<?hh // strict

function foo(): (function(inout int): int) {
  return $x ==> 42;
}
