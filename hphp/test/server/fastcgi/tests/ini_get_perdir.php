<?php

require_once(__DIR__ . '/test_base.inc');

function perdirTestController($port) {
  $host = php_uname('n');
  //
  // The server executes from its root dir,
  // namely from ../server_root/
  // and will execute ../server_root/test_ini_get_perdir.php
  //
  echo request($host, $port, 'test_ini_get_perdir.php');
  echo "\n";
}

runTest("perdirTestController");
