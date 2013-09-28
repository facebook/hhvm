<?php

require_once('test_base.php');

function test1Controller($hphpdOutput, $hphpdProcessId, $serverPort) {
  // Request a page so that the client can debug it.
  waitForClientToOutput($hphpdOutput, "Waiting for server response");
  $url = "http://".php_uname('n').':'.$serverPort.'/test1.php';
  echo "Requesting test1.php\n";
  request($url, 10); // proceed without waiting for a response

  // Let client run until script quits
  waitForClientToOutput($hphpdOutput, "quit");
}

runTest('test1', "test1Controller");
