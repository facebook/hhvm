<?php

$LOG_ROOT = '/tmp/hhvm_server';
require __DIR__ . '/../../util/server_tests.inc';

function runTest($testController, $customArgs = '') {
  $pid = posix_getpid();
  try {
    $serverPort = $adminPort = null;
    $debugPort = false;
    $serverProc = startServer($serverPort, $adminPort, $debugPort,
                              __DIR__.'/..', __DIR__.'/../server_root',
                              $customArgs);
    $testController($serverPort);
    stopServer($adminPort);
  } catch (Exception $e) {
    error_log("Caught exception, test failed, pid=$pid, exn=".$e->getMessage());
    killChildren($pid);
    error_log('test failed');
  }
}
