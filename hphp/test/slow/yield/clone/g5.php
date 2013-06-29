<?php
$a1 = 1;
$a2 = -999999999;
$foo = function () use ($a1, &$a2) {
  $a1 += 10;
  $a2 += 100;
  yield $a1 * 10000 + $a2;
  $a1 += 20;
  $a2 += 200;
  yield $a1 * 10000 + $a2;
  $a1 += 30;
  $a2 += 300;
  yield $a1 * 10000 + $a2;
};
$x = $foo();
$y1 = clone $x;
$y2 = clone $x;
$a2 = 2;
foreach ($y1 as $v) {
  $v1 = (int)($v / 10000);
  $v2 = $v % 10000;
  echo $v1 . " " . $v2 . "\n";
  ++$a2;
}
echo "--------\n";
var_dump($a1, $a2);
echo "========\n";
foreach ($y2 as $v) {
  $v1 = (int)($v / 10000);
  $v2 = $v % 10000;
  echo $v1 . " " . $v2 . "\n";
  ++$a2;
}
echo "--------\n";
var_dump($a1, $a2);
echo "========\n";
foreach ($x as $v) {
  $v1 = (int)($v / 10000);
  $v2 = $v % 10000;
  echo $v1 . " " . $v2 . "\n";
  ++$a2;
}
echo "--------\n";
var_dump($a1, $a2);
