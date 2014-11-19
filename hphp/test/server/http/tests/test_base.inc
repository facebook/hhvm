<?php

$LOG_ROOT = '/tmp/hhvm_server';
require __DIR__ . '/../../util/server_tests.inc';

function runTest($testController, $customArgs = '') {
  $pid = posix_getpid();
  try {
    $serverPort = get_random_port();
    $adminPort = get_random_port();
    while ($adminPort === $serverPort) {
      $adminPort = get_random_port();
    }

    $serverProc = null;
    $clientProcessId = 0;
    $serverProc = startServer($serverPort, $adminPort, 0,
                              __DIR__.'/..', __DIR__.'/../server_root',
                              $customArgs);
    waitForServerToGetGoing($serverPort);
    $testController($serverPort);
    stopServer($adminPort);
  } catch (Exception $e) {
    error_log("Caught exception, test failed, pid=$pid, exn=".$e->getMessage());
    killChildren($pid);
    error_log('test failed');
  }
}

function requestAll(array $requests, $customArgs = '') {
  runTest(
    function($serverPort) use ($requests) {
      foreach ($requests as $request) {
        echo "Requesting '$request'\n";
        var_dump(request(php_uname('n'), $serverPort, $request));
      }
    },
    $customArgs
  );
}
