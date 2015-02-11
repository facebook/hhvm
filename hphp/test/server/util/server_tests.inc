<?php

$test_run_id = posix_getpid();
$error_log_file = fopen("${LOG_ROOT}_test$test_run_id.log", 'w');

function tlog($str) {
  global $error_log_file;

  fwrite($error_log_file, $str);
  fwrite($error_log_file, "\n");
  // error_log($str);
}

function dumpLogFilesToStdoutAndDie() {
  global $error_log_file;
  global $test_run_id;
  global $LOG_ROOT;

  sleep(1);
  error_log('-------------------------------------------');
  error_log("Contents of '${LOG_ROOT}_test$test_run_id.log'");
  readfile("${LOG_ROOT}_test$test_run_id.log");
  echo "\n";
  error_log('-------------------------------------------');
  error_log("Contents of '${LOG_ROOT}_test_server$test_run_id.log'");
  readfile("${LOG_ROOT}_test_server$test_run_id.log");
  echo "\n";
  error_log('-------------------------------------------');
  error_log("Contents of '${LOG_ROOT}_test_server_stdout$test_run_id.log'");
  readfile("${LOG_ROOT}_test_server_stdout$test_run_id.log");
  echo "\n";
  error_log('-------------------------------------------');
  error_log("Contents of '${LOG_ROOT}_test_server_stderr$test_run_id.log'");
  readfile("${LOG_ROOT}_test_server_stderr$test_run_id.log");
  echo "\n";
  error_log('-------------------------------------------');
  error_log("Contents of '${LOG_ROOT}_test_client$test_run_id.log'");
  readfile("${LOG_ROOT}_test_client$test_run_id.log");
  echo "\n";
  error_log('-------------------------------------------');
  error_log("Contents of '${LOG_ROOT}_test_sandbox_access.log'");
  readfile("${LOG_ROOT}_test_sandbox_access.log");
  echo "\n";
  error_log('-------------------------------------------');
  error_log("Contents of '${LOG_ROOT}_curl$test_run_id.log'");
  readfile("${LOG_ROOT}_curl$test_run_id.log");
  echo "\n";
  error_log('-------------------------------------------');
  throw new Exception("test failed");
}

function hphp_home() {
  // __DIR__ == result.'hphp/test/server/util'
  return realpath(__DIR__.'/../../../..');
}

function bin_root() {
  $dir = hphp_home() . '/' . '_bin';
  return is_dir($dir) ?
    $dir :      # fbmake
    hphp_home() # github
  ;
}

function get_random_port($exclude1, $exclude2) {
  $BasePort = 20000;
  $PortRange = 3000;
  do {
    $t = rand($BasePort, $BasePort+$PortRange);
  } while ($t == $exclude1 || $t == $exclude2);
  return $t;
}

function startServer(&$serverPort, &$adminPort, &$debugPort, $home, $root,
                     $customArgs = '') {
  global $test_run_id, $LOG_ROOT;

  $chooseServer = $serverPort === null;
  $chooseAdmin = $adminPort === null;
  $chooseDebug = $debugPort === null;
  $pid = posix_getpid();

  for ($i = 0; $i < 5; $i++) {
    if ($chooseServer) $serverPort = get_random_port($adminPort, $debugPort);
    if ($chooseAdmin) $adminPort = get_random_port($serverPort, $debugPort);
    if ($chooseDebug) $debugPort = get_random_port($serverPort, $adminPort);

    $portConfig = ' -vServer.Port='.$serverPort;
    $serverConfig = ' --config='.$home.'/config/server.ini';
    $logFileConfig = ' -vLog.File='."${LOG_ROOT}_test_server$test_run_id.log";
    $logFileConfig.= ' -vLog.Access.Default.File='.
      "${LOG_ROOT}_access_log$test_run_id.log";
    $srcRootConfig = ' -vServer.SourceRoot='.$root;
    $includePathConfig = ' -vServer.IncludeSearchPaths.0='.$root;
    $adminPortConfig = $adminPort ? ' -vAdminServer.Port='.$adminPort : '';
    $debugPortConfig = $debugPort ? ' -vEval.Debugger.Port='.$debugPort : '';
    $repoConfig = " -vRepo.Central.Path=${LOG_ROOT}_server$test_run_id.hhbc";
    $useJit = array_key_exists('HHVM_JIT', $_ENV) && $_ENV['HHVM_JIT'] == 1;
    $jitConfig = ' -vEval.Jit='.($useJit ? "true" : "false");
    // To emulate sandbox setup, let Sandbox.Home be '$home'
    // and user name be 'debugger', so that the server can find the
    // sandbox_conf.hdf in '$root'.
    $sandboxHomeConfig = ' -vSandbox.Home='.$home;

    $hhvm = PHP_BINARY;

    if (ini_get('hhvm.repo.authoritative')) {
      $cmd = "$hhvm --hphp -k1 -l2 -t hhbc -o ${LOG_ROOT}_bytecode$test_run_id ".
        "--input-dir $root";
      tlog('Building repo with command: '.$cmd);
      tlog(exec($cmd));
      $repoConfig .=
        " -vRepo.Local.Path=${LOG_ROOT}_bytecode$test_run_id/hhvm.hhbc".
        " -vRepo.Authoritative=true";
    }

    $cmd = "exec env MALLOC_CONF=junk:true TESTID={$test_run_id} " . $hhvm .
      ' --mode=server' . $serverConfig . $logFileConfig .
      ' -vEval.JitProfileInterpRequests=0 -vServer.ExitOnBindFail=true' .
      ' --instance-id=' . $test_run_id .
      $portConfig . $srcRootConfig .
      $includePathConfig . $sandboxHomeConfig . $adminPortConfig .
      $debugPortConfig . $repoConfig . $jitConfig .
      ' ' . $customArgs .
      " > ${LOG_ROOT}_test_server_stdout$test_run_id.log" .
      " 2> ${LOG_ROOT}_test_server_stderr$test_run_id.log";

    tlog('Starting server with command: '.$cmd);
    $pipes = array();
    $serverProc = proc_open($cmd, array(), $pipes);
    if (!is_resource($serverProc)) {
      tlog('Failed to start a shell process for the server');
    } else if (waitForServerToGetGoing($serverPort, $serverProc)) {
      return $serverProc;
    }
    killChildren($pid);
    if ($serverProc) proc_close($serverProc);
  }
  dumpLogFilesToStdoutAndDie();
}

function waitForServerToGetGoing($serverPort, $serverProc) {
  global $test_run_id;
  $host = php_uname('n');
  $r = "";
  for ($i = 1; $i <= 20; $i++) {
    $status = proc_get_status($serverProc);
    if ($status === false || !$status['running']) {
      break;
    }
    sleep(1);
    $r = request($host, $serverPort, "hello.php");
    if (preg_match('/Hello, World!(\d+)/', $r, $matches)) {
      if ($matches[1] == $test_run_id) return true;
      tlog('a server for a different test responded');
      break;
    }
    tlog('Server replied: '.$r);
  }

  tlog('Server is not responding.');
  return false;
}

function stopServer($adminPort, $serverProc) {
  global $test_run_id, $LOG_ROOT;

  $r = "";
  for ($i = 1; $i <= 10; $i++) {
    $r = request(php_uname('n'), $adminPort,
                 'stop?instance-id=' . $test_run_id);
    if ($r == "OK") break;
    usleep(100000);
  }
  if ($r != "OK") {
    tlog("Server did not stop. Response was $r");
    dumpLogFilesToStdoutAndDie();
  }
  proc_close($serverProc);

  @unlink("${LOG_ROOT}_test$test_run_id.log");
  @unlink("${LOG_ROOT}_test_server$test_run_id.log");
  @unlink("${LOG_ROOT}_test_server_stderr$test_run_id.log");
  @unlink("${LOG_ROOT}_test_server_stdout$test_run_id.log");
  @unlink("${LOG_ROOT}_test_client$test_run_id.log");
  @unlink("${LOG_ROOT}_server$test_run_id.hhbc");
  @unlink("${LOG_ROOT}_client$test_run_id.hhbc");
  @unlink("${LOG_ROOT}_curl$test_run_id.log");
  @unlink("${LOG_ROOT}_bytecode$test_run_id/hhvm.hhbc");
  @rmdir("${LOG_ROOT}_bytecode$test_run_id");
}

function http_request($host, $port, $path, $timeout = 1200) {
  global $test_run_id, $LOG_ROOT;

  @list($path, $post, $headers) = (array)$path;
  if ($post) {
    $post = http_build_query($post);
    $post = '--data-urlencode='.urlencode($post);;
  }
  if ($headers) {
    foreach ($headers as $h => $v) {
      $s[] = "-H '$h : $v'";
    }
    $headers = implode(" ", $s);
  }
  $url = "http://$host:$port/$path";
  $host_name = "hphpd.debugger.".php_uname('n');
  $cmd = "curl $post --trace-ascii ${LOG_ROOT}_curl$test_run_id.log ".
    "--silent --connect-timeout $timeout ".
    "-H 'Host: $host_name' $headers --url \"$url\"";
  tlog("Requesting page with command: $cmd");
  if (exec($cmd, $result) === null) return null;
  return implode("\n", $result);
}

function requestAll(array $requests, $customArgs = '') {
  runTest(
    function($serverPort) use ($requests) {
      foreach ($requests as $request) {
        list($r) = (array)$request;
        echo "Requesting '$r'\n";
        var_dump(request(php_uname('n'), $serverPort, $request));
      }
    },
    $customArgs
  );
}

if (!function_exists("request")) {
  function request($host, $port, $path, $timeout = 1200) {
    return http_request($host, $port, $path, $timeout);
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
