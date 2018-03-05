<?hh // strict

function testObjGet(dynamic $x): int {
  $y = $x();
  return $y;
}
