<?hh // strict

function foo(): (function(int): int) {
  return (int $x) ==> $x;
}
