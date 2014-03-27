<?php

$test_run_id = rand(100000, 999999);
$error_log_file = fopen("/tmp/hphpd_test$test_run_id.log", 'w');

function tlog($str) {
  global $error_log_file;

  fwrite($error_log_file, $str);
  fwrite($error_log_file, "\n");
  // error_log($str);
}

function dumpLogFilesToStdoutAndDie() {
  global $error_log_file;
  global $test_run_id;

  sleep(1);
  error_log('-------------------------------------------');
  error_log("Contents of '/tmp/hphpd_test$test_run_id.log'");
  readfile("/tmp/hphpd_test$test_run_id.log");
  echo "\n";
  error_log('-------------------------------------------');
  error_log("Contents of '/tmp/hphpd_test_server$test_run_id.log'");
  readfile("/tmp/hphpd_test_server$test_run_id.log");
  echo "\n";
  error_log('-------------------------------------------');
  error_log("Contents of '/tmp/hphpd_test_server_stdout$test_run_id.log'");
  readfile("/tmp/hphpd_test_server_stdout$test_run_id.log");
  echo "\n";
  error_log('-------------------------------------------');
  error_log("Contents of '/tmp/hphpd_test_server_stderr$test_run_id.log'");
  readfile("/tmp/hphpd_test_server_stderr$test_run_id.log");
  echo "\n";
  error_log('-------------------------------------------');
  error_log("Contents of '/tmp/hphpd_test_client$test_run_id.log'");
  readfile("/tmp/hphpd_test_client$test_run_id.log");
  echo "\n";
  error_log('-------------------------------------------');
  error_log("Contents of '/tmp/hphpd_test_sandbox_access.log'");
  readfile("/tmp/hphpd_test_sandbox_access.log");
  echo "\n";
  error_log('-------------------------------------------');
  error_log("Contents of '/tmp/hphpd_curl$test_run_id.log'");
  readfile("/tmp/hphpd_curl$test_run_id.log");
  echo "\n";
  error_log('-------------------------------------------');
  throw new Exception("test failed");
}

function hphp_home() {
  // __DIR__ == result.'hphp/test/server/debugger/tests'
  return realpath(__DIR__.'/../../../../..');
}

function bin_root() {
  $dir = hphp_home() . '/' . '_bin';
  return is_dir($dir) ?
    $dir :      # fbmake
    hphp_home() # github
  ;
}

function get_random_port() {
  $BasePort = 20000;
  $PortRange = 3000;
  return rand($BasePort, $BasePort+$PortRange);
}

function startServer($serverPort, $adminPort, $debugPort) {
  global $test_run_id;

  $home = hphp_home().'/hphp/test/server/debugger';
  $portConfig = ' -vServer.Port='.$serverPort;
  $serverConfig = ' --config='.$home.'/config/debugger-server.hdf';
  $logFileConfig = ' -vLog.File='."/tmp/hphpd_test_server$test_run_id.log";
  $srcRootConfig = ' -vServer.SourceRoot='.$home.'/debugger';
  $includePathConfig = ' -vServer.IncludeSearchPaths.0='.$home.'/debugger';
  $adminPortConfig = ' -vAdminServer.Port='.$adminPort;
  $debugPortConfig = ' -vEval.Debugger.Port='.$debugPort;
  $repoConfig = " -vRepo.Central.Path=/tmp/hphpd_server$test_run_id.hhbc";
  $useJit = array_key_exists('HHVM_JIT', $_ENV) && $_ENV['HHVM_JIT'] == 1;
  $jitConfig = ' -vEval.Jit='.($useJit ? "true" : "false");
  // To emulate sandbox setup, let Sandbox.Home be '$home'
  // and user name be 'debugger', so that the server can find the sandbox_conf.hdf
  // in '$home'.'/debugger'.
  $sandboxHomeConfig = ' -vSandbox.Home='.$home;

  $hhvm = bin_root().'/hphp/hhvm/hhvm';

  $cmd = $hhvm.' --mode=server' . $serverConfig . $logFileConfig .
    ' -vEval.JitWarmupRequests=0' . $portConfig . $srcRootConfig .
    $includePathConfig . $sandboxHomeConfig . $adminPortConfig .
    $debugPortConfig . $repoConfig . $jitConfig .
     " > /tmp/hphpd_test_server_stdout$test_run_id.log" .
     " 2> /tmp/hphpd_test_server_stderr$test_run_id.log";

  tlog('Starting server with command: '.$cmd);
  $pipes = array();
  $serverProc = proc_open($cmd, array(), $pipes);
  if (!is_resource($serverProc)) {
    tlog('Failed to start a shell process for the server');
    dumpLogFilesToStdoutAndDie();
  }
  return $serverProc;
}

function waitForServerToGetGoing($serverPort) {
  global $test_run_id;

  $host = php_uname('n');
  $url = "http://$host:$serverPort/hello.php";
  $r = "";
  for ($i = 1; $i <= 20; $i++) {
    sleep(1);
    $r = request($url);
    if ($r === "Hello, World!") break;
  }

  if ($r !== "Hello, World!") {
    tlog('Server is not responding.');
    dumpLogFilesToStdoutAndDie();
  }
}

function stopServer($adminPort) {
  global $test_run_id;

  $url = "http://".php_uname('n').':'.$adminPort.'/stop';
  $r = "";
  for ($i = 1; $i <= 10; $i++) {
    $r = request($url);
    if ($r == "OK") break;
    usleep(100000);
  }
  if ($r != "OK") {
    tlog("Server did not stop. Response was $r");
    dumpLogFilesToStdoutAndDie();
  }
  unlink("/tmp/hphpd_test$test_run_id.log");
  unlink("/tmp/hphpd_test_server$test_run_id.log");
  unlink("/tmp/hphpd_test_server_stderr$test_run_id.log");
  unlink("/tmp/hphpd_test_server_stdout$test_run_id.log");
  unlink("/tmp/hphpd_test_client$test_run_id.log");
  unlink("/tmp/hphpd_server$test_run_id.hhbc");
  unlink("/tmp/hphpd_client$test_run_id.hhbc");
}

function request($url, $timeout = 1200) {
  global $test_run_id;

  $host = "hphpd.debugger.".php_uname('n');
  $cmd = "curl --trace-ascii /tmp/hphpd_curl$test_run_id.log ".
    "--silent --connect-timeout $timeout ".
    "--header 'Host: hphpd.debugger.$host' --url $url";
  tlog("Requesting page with command: $cmd");
  return exec($cmd);
}

function startDebuggerClient($debugPort, $input_path, &$pipes) {
  global $test_run_id;

  $home = hphp_home().'/hphp/test/server/debugger';
  $hhvm = bin_root().'/hphp/hhvm/hhvm';
  $host = ' -h '.php_uname('n');
  $port = ' --debug-port '.$debugPort;
  $user = ' --user debugger';
  $config = ' --config '.$home.'/config/debugger-client.hdf';
  $logFileConfig = ' -vLog.File='."/tmp/hphpd_test_client$test_run_id.log";
  $repoConfig = " -vRepo.Central.Path=/tmp/hphpd_client$test_run_id.hhbc";
  $debugConfig = ' --debug-config '.$home.'/config/hphpd.ini';

  $cmd = $hhvm.' -m debug' . $host . $port . $user .
    $config . $logFileConfig . $repoConfig . $debugConfig .
    ' <'.$home.$input_path;

  $descriptorspec = array(
     0 => array("pipe", "r"),
     1 => array("pipe", "w"),
     2 => array("pipe", "w"),
  );

  $env = $_ENV;
  $env["TERM"] = "dumb";

  tlog('Starting debugger client with command: '.$cmd);
  $process = proc_open("$cmd 2>&1", $descriptorspec, $pipes, null, $env);
  if (!is_resource($process)) {
    tlog('Failed to start a shell process for the server');
    dumpLogFilesToStdoutAndDie();
  }
}

function runTest($testName, $testController) {
  try {
    $serverPort = get_random_port();
    $adminPort = get_random_port();
    while ($adminPort === $serverPort) {
      $adminPort = get_random_port();
    }
    $debugPort = get_random_port();
    while ($debugPort === $serverPort || $debugPort === $adminPort) {
      $debugPort = get_random_port();
    }

    $pid = posix_getpid();
    $serverProc = null;
    $clientProcessId = 0;
    $serverProc = startServer($serverPort, $adminPort, $debugPort);
    waitForServerToGetGoing($serverPort);
    startDebuggerClient($debugPort, "/debugger/$testName.in", $pipes);
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
    stopServer($adminPort);
  } catch (Exception $e) {
    error_log("Caught exception, test failed, pid=$pid");
    killChildren(posix_getpid());
    error_log('test failed');
  }
}

function killChildren($pid) {
  $childIds = exec("pgrep -f -d , -P $pid");
  foreach (explode(",", $childIds) as $cid) {
    if (!$cid) continue;
    tlog("killing ".exec("ps -f -p ".$cid));
    killChildren($cid);
    posix_kill($cid, SIGKILL);
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
  global $test_run_id;

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

