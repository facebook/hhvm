<?hh

function foo(): (function(int): int) {
  return (int $x) ==> $x;
}
