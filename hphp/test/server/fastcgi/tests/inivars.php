<?php

require_once(__DIR__ . '/test_base.inc');

function inivarsTestController($port) {
  $host = php_uname('n');
  //
  // The server executes from its root dir,
  // namely from ../server_root/
  // and will execute ../server_root/test_inivars.php
  //
  echo request($host, $port, 'test_inivars.php');
  echo "\n";
}

runTest("inivarsTestController");
