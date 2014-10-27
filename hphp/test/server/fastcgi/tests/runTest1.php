<?php

require_once('test_base.inc');

function test1Controller($serverPort) {
  $requests = array(
    "test_get.php?name=Foo",
    "test_get.php?name=Bar",
    "subdoc/subdir/test.php",
  );
  foreach ($requests as $request) {
    echo "Requesting '$request'\n";
    var_dump(request(php_uname('n'), $serverPort, $request));
  }
}

runTest('test1', "test1Controller");
