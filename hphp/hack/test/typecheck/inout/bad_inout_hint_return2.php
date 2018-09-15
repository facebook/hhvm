<?hh // strict

function foo(): (function(int): int) {
  return (inout $x) ==> 42;
}
