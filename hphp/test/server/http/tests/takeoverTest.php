<?hh

require __DIR__ . '/../../util/server_tests.inc';
ServerUtilServerTests::$LOG_ROOT = '/tmp/hhvm_server';

function runTakeoverTest() {

  $pid = posix_getpid();
  $takeoverFile = '/tmp/takeover.'.ServerUtilServerTests::$test_run_id;
  $serverProc = $serverPort = $adminPort = null;
  $debugPort = false;
  $serverHome = __DIR__.'/..';
  $serverRoot = __DIR__.'/../server_root';
  $customArgs = " -vServer.TakeoverFilename={$takeoverFile}";

  try {
    $serverProc = startServer(&$serverPort, &$adminPort, &$debugPort,
                              $serverHome, $serverRoot,
                              $customArgs);
    if ($serverProc === null) {
      echo('failed to start first server');
      return;
    }
    $takeoverid = 'new'.ServerUtilServerTests::$test_run_id;
    $customArgs = '';
    $newServerProc = takeoverOldServer($serverPort, $adminPort,
                                       $serverHome, $serverRoot,
                                       $takeoverFile, $serverProc,
                                       $customArgs, $takeoverid);
    if ($newServerProc === null) {
      echo('failed to start another server to takeover');
      return;
    }
    // Check and make sure that the server is always online, and old
    // server exits after a finite amount of time.  It is OK if both
    // servers are working at the same time.
    $testids = array(ServerUtilServerTests::$test_run_id, $takeoverid);
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
        // old server is stuck after takeover.  5 minutes should be
        // enough even for a debug build on a slow machine.
        error_log('old server still running after 300 seconds?');
        return;
      }
      sleep(1);
    }
    if (!checkServerId($serverPort, $takeoverid)) {
      return;
    }
    stopServer($adminPort, $newServerProc);
  } catch (Exception $e) {
    error_log("Caught exception, test failed, pid=$pid, exn=".$e->getMessage());
    killChildren($pid);
    if ($serverProc) proc_close($serverProc);
    if ($newServerProc) proc_close($newServerProc);
    error_log('test failed');
    return;
  }
  echo 'takeover successful';
}

runTakeoverTest();
