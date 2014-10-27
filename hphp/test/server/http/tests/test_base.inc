<?php

$LOG_ROOT = '/tmp/hhvm_server';
require __DIR__ . '/../../util/server_tests.inc';

function runTest($testName, $testController) {
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
                              __DIR__.'/..', __DIR__.'/../server_root');
    waitForServerToGetGoing($serverPort);
    $testController($serverPort);
    stopServer($adminPort);
  } catch (Exception $e) {
    error_log("Caught exception, test failed, pid=$pid, exn=".$e->getMessage());
    killChildren($pid);
    error_log('test failed');
  }
}
