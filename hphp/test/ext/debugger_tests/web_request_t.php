<?php
require_once("hphpd.php");
require_once("hphpd_test_inc.php");
include 'break_t.php';

function test_break() {
  $x = 'test_break() in web_request_t.php';
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
  while ($a == 1) {
    sleep(1);
  }
  return $a;
}

// The test harness has started executing this program, but we need to ensure
// the debugger client the harness has also started is ready for the program to
// proceed to the first interrupt site.
waitForClientToGetBusy("web_request");

test_break();
test_sleep();
echo "request done";
