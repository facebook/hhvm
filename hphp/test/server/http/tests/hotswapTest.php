<?hh

function runHotswapTest() {

  $pid = posix_getpid();

  $serverProc = $newServerProc = $serverPort = $adminPort = null;
  $debugPort = false;
  $serverHome = __DIR__.'/..';
  $serverRoot = __DIR__.'/../server_root';
  $customArgs = " -vServer.StopOld=true";

  try {
    $serverProc = startServer(inout $serverPort, inout $adminPort, inout $debugPort,
                              $serverHome, $serverRoot,
                              $customArgs);
    if ($serverProc === null) {
      echo('failed to start first server');
      return;
    }
    $newServerId = 'new'.ServerUtilServerTests::test_run_id();
    $newServerProc = startServer(inout $serverPort, inout $adminPort, inout $debugPort,
                              $serverHome, $serverRoot,
                              $customArgs, $newServerId);
    if ($newServerProc === null) {
      echo('failed to start another server');
      return;
    }
    // Check and make sure that the server is always online, and old
    // server exits after a finite amount of time.  It is OK if both
    // servers are working at the same time.
    $testids = vec[ServerUtilServerTests::test_run_id(), $newServerId];
    for ($i = 1; ; $i++) {
      if (!checkServerId($serverPort, $testids)) {
        return;
      }
      // Old server should die after a while
      $status = proc_get_status($serverProc);
      if ($status === false) {
        error_log('error retrieving old server status');
        proc_close($serverProc);
        return;
      }
      if (!$status['running']) break;   // old server is gone
      if ($i > 300) {
        // Either the new process is initializing too slowly, or the
        // old server is stuck.  5 minutes should be
        // enough even for a debug build on a slow machine.
        error_log('old server still running after 300 seconds?');
        return;
      }
      sleep(1);
    }
    if (!checkServerId($serverPort, $newServerId)) {
      return;
    }
    stopServer($adminPort, $newServerProc);
  } catch (Exception $e) {
    error_log("Caught exception, test failed, pid=$pid, exn=\n".(string)$e);
    killChildren($pid);
    if ($serverProc) proc_close($serverProc);
    if ($newServerProc) proc_close($newServerProc);
    error_log('test failed');
    exit(-1);
  }
  echo 'hotswap successful';
}

<<__EntryPoint>>
function main() {
  require __DIR__ . '/../../util/server_tests.inc';
  ServerUtilServerTests::$LOG_ROOT = ServerUtilServerTests::working_dir() . '/hhvm_server';
  runHotswapTest();
}
