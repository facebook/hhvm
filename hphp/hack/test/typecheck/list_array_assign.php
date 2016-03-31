<?hh // strict

function test(array<int> $v): (array<int>, int, int) {
  $a = list($x, $y) = $v; // list returns an array, so the $a = list() is ok.
  return tuple($a, $x, $y);
}
