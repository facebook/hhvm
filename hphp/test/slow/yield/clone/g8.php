<?php
function foo() {
  static $x = 1;
  $x += 10;
  yield $x;
  $x += 100;
  yield $x;
  $x += 1000;
  yield $x;
}
$x = foo();
$x->next();
$y1 = clone $x;
$y2 = clone $x;
foreach ($x as $v) {
  echo $v . "\n";
}
echo "========\n";
foreach ($y1 as $v) {
  echo $v . "\n";
}
echo "========\n";
foreach ($y2 as $v) {
  echo $v . "\n";
}
