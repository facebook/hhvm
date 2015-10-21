<?php

function foo($a, &$b) {
  $var1 = 'x';
  $var2 = 'y';
  $$var1 = $a;
  $$var2 =& $b;
  unset($a);
  unset($b);
  $$var1 += 10;
  $$var2 += 100;
  yield $$var1 * 10000 + $$var2;
  $$var1 += 20;
  $$var2 += 200;
  yield $$var1 * 10000 + $$var2;
  $$var1 += 30;
  $$var2 += 300;
  yield $$var1 * 10000 + $$var2;
}
$a1 = 1;
$a2 = -999999999;
$x = foo($a1, $a2);
$a2 = 2;
$x->next();
$y1 = clone $x;
$y2 = clone $x;
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
