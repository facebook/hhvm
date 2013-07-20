<?php
include 'break1.php';

function test_break() {
  $x = 'test_break() in test1.php';
  foo($x);
  foo($x);
  $obj = new cls();
  $obj->pubObj($x);
  cls::pubCls($x);
  $obj->pubHardBreak($x);
}

function test_sleep() {
  $a = 1;
  // $a will be set to 0 by debugger after interrupt
  while ($a == 1) { sleep(1); }
  return $a;
}

test_break();
test_sleep();
echo "request done\n";
