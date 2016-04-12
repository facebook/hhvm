<?hh // strict

function test(Pair<int, string> $v): (Pair<int, string>, int, string) {
  $a = list($x, $y) = $v; // list returns an array, so the $a = list() is ok.
  return tuple($a, $x, $y);
}
