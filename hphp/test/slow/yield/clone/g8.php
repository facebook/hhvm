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
for ($x->next(); $x->valid(); $x->next()) {
  echo $x->current() . "\n";
}
echo "========\n";
for ($y1->next(); $y1->valid(); $y1->next()) {
  echo $y1->current() . "\n";
}
echo "========\n";
for ($y2->next(); $y2->valid(); $y2->next()) {
  echo $y2->current() . "\n";
}
