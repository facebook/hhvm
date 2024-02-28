<?hh

function init() {
  require __DIR__ . '/../../util/server_tests.inc';
  ServerUtilServerTests::$LOG_ROOT = ServerUtilServerTests::working_dir() . '/hphpd';
}

function startDebuggerClient($debugPort, $input_path, inout $pipes) {
  $LOG_ROOT = ServerUtilServerTests::$LOG_ROOT;
  $test_run_id = ServerUtilServerTests::test_run_id();


  $home = hphp_home().'/hphp/test/server/debugger';
  $hhvm = HH\__internal\hhvm_binary();
  $host = ' -h localhost';
  $port = ' --debug-port '.$debugPort;
  $user = ' --user debugger';
  $config = ' --config '.$home.'/config/client.ini';
  $logFileConfig = ' -vLog.File='."{$LOG_ROOT}_test_client$test_run_id.log";
  $repoConfig = " -vRepo.Path={$LOG_ROOT}_client$test_run_id.hhbc";
  $debugConfig = ' --debug-config '.$home.'/config/hphpd.ini';

  $cmd = $hhvm.' -m debug' . $host . $port . $user .
    $config . $logFileConfig . $repoConfig . $debugConfig .
    ' <'.$home.$input_path;

  $descriptorspec = dict[
     0 => vec["pipe", "r"],
     1 => vec["pipe", "w"],
     2 => vec["pipe", "w"],
  ];

  $env = $_ENV;
  $env["TERM"] = "dumb";

  tlog('Starting debugger client with command: '.$cmd);
  $process = proc_open("$cmd 2>&1", $descriptorspec, inout $pipes, null, $env);
  if (!is_resource($process)) {
    tlog('Failed to start a shell process for the server');
    dumpLogFilesToStdoutAndDie();
  }
}

function runTest($testName, $testController) {
  try {
    $pid = posix_getpid();
    $serverProc = null;
    $clientProcessId = 0;

    $serverPort = $adminPort = $debugPort = null;
    $serverProc = startServer(inout $serverPort, inout $adminPort, inout $debugPort,
                              __DIR__.'/..', __DIR__.'/../debugger',
                              '-vHack.Lang.OptimizeLocalLifetimes=0');
    $pipes = null;
    startDebuggerClient($debugPort, "/debugger/$testName.in", inout $pipes);
    $clientProcessId = getClientProcessId($pipes[1]);
    if (!$clientProcessId ||
        ($clientProcessId = intval($clientProcessId)) <= 0) {
      tlog('Failed to communicate with the debugger client process');
      dumpLogFilesToStdoutAndDie();
    }
    tlog("Debugger client process id = $clientProcessId");
    $testController($pipes[1], $clientProcessId, $serverPort);
    // Echo stderr, just in case.
    // (It was redirected to stdout, so this should be empty).
    echo stream_get_contents($pipes[2]);
    stopServer($adminPort, $serverProc);
  } catch (Exception $e) {
    error_log("Caught exception, test failed, pid=$pid, exn=\n".(string)$e);
    killChildren(posix_getpid());
    if ($serverProc) proc_close($serverProc);
    error_log('test failed');
    exit(-1);
  }
}

function getClientProcessId($pipe) {
  tlog("reading initial client output for client process id");
  while (!feof($pipe)) {
    $clientOutput = fgets($pipe);
    tlog($clientOutput);
    if (strpos($clientOutput, "running in script mode, pid=") === 0) {
      return substr($clientOutput, 28);
    }
  }
  if (feof($pipe)) tlog("client closed the pipe.");
  tlog("done reading client output for client process id");
}

function waitForClientToOutput($pipe, $string1, $retryCount = 20) {


  tlog("reading client output");
  $rc = $retryCount;
  while (!feof($pipe)) {
    $clientOutput = fgets($pipe);
    tlog($clientOutput);
    if (strpos($clientOutput,
        ".....Debugger client still waiting for server response.....") === 0) {
      if (--$rc > 0) continue;
      dumpLogFilesToStdoutAndDie();
    }
    echo $clientOutput;
    if (strpos($clientOutput, $string1) === 0) break;
    $rc = $retryCount;
  }
  if (feof($pipe)) tlog("client closed the pipe.");
  tlog("done reading client output");
}
