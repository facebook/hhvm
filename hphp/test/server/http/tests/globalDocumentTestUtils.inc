<?hh

function runTestWith404HealthCheck($testController, $customArgs = '', $repoArgs = '',
                                   $expect404 = false) {
  $pid = posix_getpid();
  try {
    $serverProc = $serverPort = $adminPort = null;
    $debugPort = false;
    $serverProc = startServer(inout $serverPort, inout $adminPort, inout $debugPort,
                              __DIR__.'/..', __DIR__.'/../server_root',
                              $customArgs, null, $repoArgs, true);
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
