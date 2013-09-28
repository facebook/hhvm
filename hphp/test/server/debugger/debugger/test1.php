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

test_break();
echo "request done\n";
