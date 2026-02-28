<?hh

function init() {
  require __DIR__ . '/../../util/server_tests.inc';
  ServerUtilServerTests::$LOG_ROOT = ServerUtilServerTests::working_dir() . '/hhvm_server';
}

function runTest($testController, $customArgs = '', $repoArgs = '') {
  $pid = posix_getpid();
  try {
    $serverProc = $serverPort = $adminPort = null;
    $debugPort = false;
    $serverProc = startServer(inout $serverPort, inout $adminPort, inout $debugPort,
                              __DIR__.'/..', __DIR__.'/../server_root',
                              $customArgs, null, $repoArgs);
    $testController($serverPort);
    stopServer($adminPort, $serverProc);
  } catch (Exception $e) {
    error_log("Caught exception, test failed, pid=$pid, exn=\n".(string)$e);
    killChildren($pid);
    if ($serverProc) proc_close($serverProc);
    error_log('test failed');
    exit(-1);
  }
}
