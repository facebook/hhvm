<?hh // strict

function testObjGet(dynamic $x): int {
  $y = $x->y;
  return $y;
}
