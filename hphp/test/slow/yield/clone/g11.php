<?php
function f(&$a) {
  ++$a;
  yield $a;
  ++$a;
  yield $a;
}
$a = 3;
$x = f($a);
unset($a);
$y1 = clone $x;
$y2 = clone $x;
foreach ($y1 as $v) {
  echo $v . "\n";
}
echo "========\n";
foreach ($y2 as $v) {
  echo $v . "\n";
}
echo "========\n";
foreach ($x as $v) {
  echo $v . "\n";
}
