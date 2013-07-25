<?php

require_once('test_base.php');

function test1Controller($hphpdOutput, $hphpdProcessId, $serverPort) {
  // Request a page so that the client can debug it.
  waitForClientToOutput($hphpdOutput, "Waiting for server response");
  $url = "http://".php_uname('n').':'.$serverPort.'/test1.php';
  echo "Requesting test1.php\n";
  request($url, 10); // proceed without waiting for a response

  // Send ctrl-c to client, which is waiting for a hung server
  waitForClientToOutput($hphpdOutput, "Break at cls::pubHardBreak()", $url);
  waitForClientToOutput($hphpdOutput, "Waiting for server response");
  sleep(2); // give server a chance to get itself in the enless loop
  echo "sending SIGINT to the debugger client.\n";
  posix_kill($hphpdProcessId, SIGINT);

  // Let client run until script quits
  waitForClientToOutput($hphpdOutput, "quit");
}

runTest('test1', "test1Controller");
