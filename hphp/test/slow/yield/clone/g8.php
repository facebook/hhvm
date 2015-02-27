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
$x->rewind();
$y1 = clone $x;
$y2 = clone $x;
$x->next();
foreach ($x as $v) {
  echo $v . "\n";
}
echo "========\n";
$y1->next();
foreach ($y1 as $v) {
  echo $v . "\n";
}
echo "========\n";
$y2->next();
foreach ($y2 as $v) {
  echo $v . "\n";
}
